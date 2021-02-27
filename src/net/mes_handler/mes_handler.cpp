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
