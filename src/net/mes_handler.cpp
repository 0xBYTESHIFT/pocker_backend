#include "net/mes_handler.h"
#include "net/tcp_connection.h"

void mes_handler::add(tcp_connection_ptr conn) {
    participants_.insert(conn);
    conn->add_handler(shared_from_this());
    conn->start();
}

void mes_handler::del(tcp_connection_ptr conn) {
    participants_.erase(conn);
}

void mes_handler::broadcast(const message& msg) {
    for (auto participant : participants_) {
        participant->deliver(msg);
    }
}