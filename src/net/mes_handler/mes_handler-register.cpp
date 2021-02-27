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
        auto rehashed_pass = hash_(req_pass + db_wrkr_->config().db_pass_salt);
        db(insert_into(table).set(table.email = req_email, table.pass_hash = rehashed_pass,
                                  table.nickname = req_nick, table.user_id = id_val));
        rsp.code = decltype(rsp)::code_enum::OK;
        rsp.message = "registration success";
    }
    message msg = rsp.to_json();
    sender->deliver(std::move(msg));
}