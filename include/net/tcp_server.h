#pragma once
#include "net/tcp_connection.h"

#include "net/mes_handler.h"
#include <boost/asio.hpp>
#include <boost/asio/ip/tcp.hpp>

class tcp_server {
public:
    using io_context_t = tcp_connection::io_context_t;
    using ec_t = tcp_connection::ec_t;
    using pointer_t = tcp_connection::pointer_t;

    tcp_server(io_context_t& io_context, size_t port);

    auto get_mes_handler() const -> std::shared_ptr<mes_handler>;

private:
    void start_accept();
    void handle_accept(pointer_t new_connection, const ec_t& ec);

    using tcp = boost::asio::ip::tcp;

    io_context_t& io_context_;
    tcp::acceptor acceptor_;
    std::shared_ptr<mes_handler> handler_;
};