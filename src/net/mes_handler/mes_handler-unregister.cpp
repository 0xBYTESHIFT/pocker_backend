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

void mes_handler::handle_unreg_request_(const json& j, tcp_connection_ptr sender) {
    ZoneScoped;
    auto lgr = get_logger();
    auto prefix = "mes_handler::handle_unreg_request_";

    api::unregister_request req;
    req.from_json(j);
    auto& db_ptr = this->db_wrkr_->db();

    tab_user table;
    auto& req_email = req.email().downcast();
    auto req_pass = hash_(req.pass_hash().downcast() + db_wrkr_->config().db_pass_salt);
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