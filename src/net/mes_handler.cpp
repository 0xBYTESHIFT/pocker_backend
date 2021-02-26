#include "net/mes_handler.h"
#include "components/api/api.h"
#include "db/database_worker.h"
#include "db/table_user.hpp"
#include "json_obj.h"
#include "log.hpp"
#include "net/tcp_connection.h"
#include "server.h"
#include "tracy_include.h"
#include "user.h"
#include <boost/algorithm/hex.hpp>
#include <iostream>
#include <openssl/ssl3.h>

static auto hash(const std::string& orig) -> std::string {
    ZoneScoped;
    std::string result(SHA512_DIGEST_LENGTH, ' ');
    SHA512(reinterpret_cast<const unsigned char*>(orig.data()), orig.size(),
           reinterpret_cast<unsigned char*>(result.data()));
    return boost::algorithm::hex(result);
}

void mes_handler::add_connection(const tcp_connection_ptr& conn) {
    ZoneScoped;
    participants_.insert(conn);
    conn->add_handler(shared_from_this());
    conn->start();
}

void mes_handler::del_connection(const tcp_connection_ptr& conn) {
    ZoneScoped;
    participants_.erase(conn);
}

void mes_handler::set_server(const server_ptr& srv) {
    ZoneScoped;
    this->srv_ = srv;
}
auto mes_handler::get_server() const -> const server_ptr& {
    ZoneScoped;
    return this->srv_;
}
auto mes_handler::get_server() -> server_ptr& {
    ZoneScoped;
    return this->srv_;
}

void mes_handler::handle(const message& msg, tcp_connection_ptr sender) {
    ZoneScoped;
    auto lgr = get_logger();
    auto prefix = "mes_handler::handle";
    std::string str(msg.body(), msg.body_length());
    json j = str;
    auto val = j.value_as<std::string>("type");

    if (val == api::register_request::type().downcast()) {
        handle_reg_request_(j, sender);
    } else if (val == api::login_request::type().downcast()) {
        handle_login_request_(j, sender);
    } else if (val == api::unregister_request::type().downcast()) {
        handle_unreg_request_(j, sender);
    } else {
        lgr.debug("{} got message:{}", prefix, str);
    }
}

void mes_handler::connect_db_worker(const database_worker_ptr& wrkr) {
    ZoneScoped;
    this->db_wrkr_ = wrkr;
}

void mes_handler::handle_unreg_request_(const json& j, tcp_connection_ptr sender) {
    ZoneScoped;
    auto lgr = get_logger();
    auto prefix = "mes_handler::handle_unreg_request_";

    api::unregister_request req;
    req.from_json(j);
    auto& db_ptr = this->db_wrkr_->db();

    tab_user table;
    auto& req_email = req.email().downcast();
    auto req_pass = hash(req.pass_hash().downcast() + db_wrkr_->config().db_pass_salt);
    lgr.debug("{} got unreg request, email:{} pass:{}", prefix, req_email, req_pass);

    auto& db = *db_ptr;
    auto results = db(select(table.email, table.pass_hash)
                          .from(table)
                          .where(table.email == req_email));
    bool email_ok = false;
    api::unregister_response rsp;
    for (const auto& row : results) {
        if (row.email == req_email) {
            email_ok = true;
            if (row.pass_hash == req_pass) {
                lgr.debug("{} unreg success, email:{}", prefix, req_email);
                db(remove_from(table).where(table.email == req_email));
                rsp.code = decltype(rsp)::code_enum::OK;
                rsp.message() = "unregister processed, status:ok";
                break;
            } else {
                lgr.debug("{} unreg unsuccess, pass error, email:{}", prefix, req_email);
                rsp.code = decltype(rsp)::code_enum::NO_PASS;
                rsp.message() = "invalid pass";
                break;
            }
        }
    }
    if (!email_ok) {
        lgr.debug("{} unreg failed, no email {}", prefix, req_email);
        rsp.code = decltype(rsp)::code_enum::NO_NAME;
        rsp.message = "invalid email";
    }
    message msg = rsp.to_json();
    sender->deliver(std::move(msg));
}

void mes_handler::handle_reg_request_(const json& j, tcp_connection_ptr sender) {
    ZoneScoped;
    auto lgr = get_logger();
    auto prefix = "mes_handler::handle_reg_request_";

    api::register_request req;
    req.from_json(j);
    auto& db_ptr = this->db_wrkr_->db();

    tab_user table;
    auto& req_email = req.email().downcast();
    auto& req_pass = req.pass_hash().downcast();
    auto& req_nick = req.nickname().downcast();
    lgr.debug("{} got request, email:{} nick:{} pass:{}", prefix, req_email, req_nick, req_pass);

    auto& db = *db_ptr;
    auto results = db(select(table.email, table.pass_hash)
                          .from(table)
                          .where(table.email == req_email));
    bool ok = true;
    api::register_response rsp;
    for (const auto& row : results) {
        if (row.email == req_email) {
            lgr.debug("{} email {} is already in use", prefix, req_email);
            rsp.code = decltype(rsp)::code_enum::NAME_TAKEN;
            rsp.message() = "email is already in use";
            ok = false;
            break;
        }
    }
    if (ok) {
        auto max_id = db(select(max(table.user_id))
                             .from(table)
                             .unconditionally());
        auto id_val = max_id.front().max + 1;
        lgr.debug("{} adding registration to db, user id:{}", prefix, id_val);
        auto rehashed_pass = hash(req_pass + db_wrkr_->config().db_pass_salt);
        db(insert_into(table).set(table.email = req_email, table.pass_hash = rehashed_pass,
                                  table.nickname = req_nick, table.user_id = id_val));
        rsp.code = decltype(rsp)::code_enum::OK;
        rsp.message = "registration success";
    }
    message msg = rsp.to_json();
    sender->deliver(std::move(msg));
}

void mes_handler::handle_login_request_(const json& j, tcp_connection_ptr sender) {
    ZoneScoped;
    auto lgr = get_logger();
    auto prefix = "mes_handler::handle_login_request_";
    api::login_request req;
    req.from_json(j);
    auto& db_ptr = this->db_wrkr_->db();

    tab_user table;
    auto& req_email = req.email().downcast();
    auto req_pass = hash(req.pass_hash().downcast() + db_wrkr_->config().db_pass_salt);
    auto& db = *db_ptr;
    auto results = db(select(table.user_id, table.nickname, table.email, table.pass_hash)
                          .from(table)
                          .where(table.email == req_email));
    lgr.debug("{} got login request, email:{} pass:{}", prefix, req_email, req_pass);
    api::login_response rsp;
    if (results.empty()) {
        rsp.code = decltype(rsp)::code_enum::NAME_UNKNOWN;
        rsp.message = "email is unkown";
        lgr.debug("{} email {} is unknown", prefix, req_email);
    } else {
        bool status = false;
        for (auto& row : results) {
            if (row.pass_hash == req_pass) {
                //TODO:check if already logined in
                rsp.code = decltype(rsp)::code_enum::OK;
                rsp.message = "login success";
                lgr.debug("{} email {} successfuly logined in", prefix, req_email);
                status = true;

                auto user = std::make_shared<class user>(row.user_id, sender);
                user->email = row.email;
                user->nickname = row.nickname;
                this->srv_->add_user(user);
                break;
            }
        }
        if (!status) {
            rsp.code = decltype(rsp)::code_enum::PASS_INCORRECT;
            lgr.debug("{} email {} not login in, wrong pass", prefix, req_email);
            rsp.message = "password is incorrect";
        }
    }
    message msg = rsp.to_json();
    sender->deliver(std::move(msg));
}