#include "net/mes_handler.h"
#include "api.h"
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

    if(val == api::register_request::type.get().downcast()){
        api::register_request req;
        req.from_json(j);
        std::cout << "got register_req:" << req.to_json() << std::endl;
    }
}