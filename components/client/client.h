#pragma once
#include "net/message.hpp"
#include <boost/asio.hpp>
#include <boost/lockfree/spsc_queue.hpp>
#include <chrono>
#include <deque>
#include <optional>
#include <readerwriterqueue/readerwriterqueue.h>

using boost::asio::ip::tcp;
class client {
public:
    using message_queue = std::deque<message>;
    using io_context_t = boost::asio::io_context;

    client(io_context_t& io_context,
           const tcp::resolver::results_type& endpoints);

    void write(const message& msg);
    void close();

    auto read_avail() -> size_t;
    auto read_if_avail() -> std::optional<message>;
    auto read_wait() -> message;
    auto read_wait(std::chrono::milliseconds ms) -> std::optional<message>;

private:
    void do_connect_(const tcp::resolver::results_type& endpoints);
    void do_read_header_();
    void do_read_body_();
    void do_write_();

private:
    boost::asio::io_context& io_context_;
    tcp::socket socket_;

    message read_msg_;
    message_queue write_msgs_;
    moodycamel::BlockingReaderWriterQueue<message> read_messages_;
};