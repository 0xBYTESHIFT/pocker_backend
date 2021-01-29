#pragma once
#include "net/message.hpp"
#include <boost/asio.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>
#include <deque>

class mes_handler;

class tcp_connection : public std::enable_shared_from_this<tcp_connection> {
public:
    using mes_queue = std::deque<message>;
    using pointer_t = std::shared_ptr<tcp_connection>;
    using socket_t = boost::asio::ip::tcp::socket;
    using io_context_t = boost::asio::io_context;
    using ec_t = boost::system::error_code;

    static auto create(io_context_t& io_context) -> pointer_t;

    void add_handler(std::shared_ptr<mes_handler> handler);
    void start();
    void deliver(const message& msg);
    auto socket() -> socket_t&;

private:
    tcp_connection(io_context_t& io_context);

    void do_read_header_();
    void do_read_body_();
    void do_write_();

    socket_t socket_;
    std::shared_ptr<mes_handler> handler_;
    message read_msg_;
    mes_queue write_msgs_;
};
using tcp_connection_ptr = std::shared_ptr<tcp_connection>;