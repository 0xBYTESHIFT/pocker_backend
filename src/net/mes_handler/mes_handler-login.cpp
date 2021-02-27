#include "components/api/api.h"
#include "db/database_worker.h"
#include "db/table_user.hpp"
#include "json_obj.h"
#include "log.hpp"
#include "net/mes_handler.h"
#include "net/tcp_connection.h"
#include "server.h"
#include "tracy_include.h"
#include "user.h"

void mes_handler::handle_login_request_(const json& j, tcp_connection_ptr sender) {
    ZoneScoped;
    auto lgr = get_logger();
    auto prefix = "mes_handler::handle_login_request_";
    api::login_request req;
    req.from_json(j);
    auto& db_ptr = this->db_wrkr_->db();

    tab_user table;
    auto& req_email = req.email().downcast();
    auto req_pass = hash_(req.pass_hash().downcast() + db_wrkr_->config().db_pass_salt);
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