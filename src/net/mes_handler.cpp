#include "net/mes_handler.h"
#include "api.h"
#include "db/database_worker.h"
#include "db/table_user.hpp"
#include "json_obj.h"
#include "log.hpp"
#include "net/tcp_connection.h"
#include <boost/algorithm/hex.hpp>
#include <iostream>
#include <openssl/ssl3.h>

static auto hash(const std::string& orig) -> std::string {
    std::string result(SHA512_DIGEST_LENGTH, ' ');
    SHA512(reinterpret_cast<const unsigned char*>(orig.data()), orig.size(),
           reinterpret_cast<unsigned char*>(result.data()));
    return boost::algorithm::hex(result);
}

void mes_handler::add(tcp_connection_ptr conn) {
    participants_.insert(conn);
    conn->add_handler(shared_from_this());
    conn->start();
}

void mes_handler::del(tcp_connection_ptr conn) {
    participants_.erase(conn);
}

void mes_handler::handle(const message& msg, tcp_connection_ptr sender) {
    auto lgr = get_logger();
    auto prefix = "mes_handler::handle";
    std::string str(msg.body(), msg.body_length());
    json j = str;
    auto val = j.value_as<std::string>("type");

    if (val == api::register_request::type().downcast()) {
        handle_reg_request_(j, sender);
    } else if (val == api::login_request::type().downcast()) {
        handle_login_request_(j, sender);
    } else {
        lgr.debug("{} got message:{}", prefix, str);
    }
}

void mes_handler::connect_db_worker(std::shared_ptr<database_worker> wrkr) {
    this->db_wrkr_ = std::move(wrkr);
}

void mes_handler::handle_reg_request_(const json& j, tcp_connection_ptr sender) {
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
    auto lgr = get_logger();
    auto prefix = "mes_handler::handle_login_request_";
    api::login_request req;
    req.from_json(j);
    auto& db_ptr = this->db_wrkr_->db();

    tab_user table;
    auto& req_email = req.email().downcast();
    auto req_pass = hash(req.pass_hash().downcast() + db_wrkr_->config().db_pass_salt);
    auto& db = *db_ptr;
    auto results = db(select(table.email, table.pass_hash)
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