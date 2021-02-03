#pragma once
#include "net/message.hpp"
#include <memory>
#include <set>

class tcp_connection;
using tcp_connection_ptr = std::shared_ptr<tcp_connection>;

class mes_handler : public std::enable_shared_from_this<mes_handler> {
public:
    using pointer_t = std::shared_ptr<mes_handler>;

    mes_handler() = default;

    void add(tcp_connection_ptr conn);
    void del(tcp_connection_ptr conn);
    void broadcast(const message& msg, tcp_connection_ptr sender = nullptr);

private:
    std::set<tcp_connection_ptr> participants_;
};