#include "net/mes_handler.h"
#include "api.h"
#include "db/database_worker.h"
#include "db/table_user.hpp"
#include "json_obj.h"
#include "net/tcp_connection.h"
#include <iostream>

void mes_handler::add(tcp_connection_ptr conn) {
    participants_.insert(conn);
    conn->add_handler(shared_from_this());
    conn->start();
}

void mes_handler::del(tcp_connection_ptr conn) {
    participants_.erase(conn);
}

void mes_handler::handle(const message& msg, tcp_connection_ptr sender) {
    std::string str(msg.body(), msg.body_length());
    std::cout << "got mes:" << str << std::endl;
    json j = str;
    auto val = j.value_as<std::string>("type");

    if (val == api::register_request::type.get().downcast()) {
        api::register_request req;
        req.from_json(j);
        std::cout << "got register_req:" << req.to_json() << std::endl;
        auto& db_ptr = this->db_wrkr_->db();

        tab_user table;
        auto results = db_ptr->operator()(select(table.first_name, table.last_name, table.user_id, table.coins)
                                              .from(table)
                                              .unconditionally());
        for (const auto& row : results) {
            std::string name = row.first_name;
            std::size_t id = row.user_id;
            std::size_t coins = row.coins;
            std::cout << "row: name: " << name << " id: " << id << " coins: " << coins << std::endl;
        }
    }
}
void mes_handler::connect_db_worker(std::shared_ptr<database_worker> wrkr) {
    this->db_wrkr_ = std::move(wrkr);
}