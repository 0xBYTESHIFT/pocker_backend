#pragma once
#include "json_obj.h"
#include "net/message.hpp"
#include <memory>
#include <set>

class database_worker;
using database_worker_ptr = std::shared_ptr<database_worker>;
class tcp_connection;
using tcp_connection_ptr = std::shared_ptr<tcp_connection>;
class server;
using server_ptr = std::shared_ptr<server>;

class mes_handler : public std::enable_shared_from_this<mes_handler> {
public:
    using pointer_t = std::shared_ptr<mes_handler>;

    mes_handler() = default;

    void add_connection(const tcp_connection_ptr& conn);
    void del_connection(const tcp_connection_ptr& conn);

    void set_server(const server_ptr& srv);
    auto get_server() const -> const server_ptr&;
    auto get_server() -> server_ptr&;

    void handle(const message& msg, tcp_connection_ptr sender = nullptr);
    void connect_db_worker(const database_worker_ptr& wrkr);

private:
    std::set<tcp_connection_ptr> participants_;
    database_worker_ptr db_wrkr_;
    server_ptr srv_;

    void handle_reg_request_(const json& j, tcp_connection_ptr sender);
    void handle_unreg_request_(const json& j, tcp_connection_ptr sender);
    void handle_login_request_(const json& j, tcp_connection_ptr sender);
};