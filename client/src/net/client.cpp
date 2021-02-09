#include "net/client.h"
#include <boost/thread.hpp>

client::client(io_context_t& io_context,
               const tcp::resolver::results_type& endpoints)
    : io_context_(io_context)
    , socket_(io_context) {
    do_connect_(endpoints);
}

auto client::read_wait() -> message {
    message msg_;
    read_messages_.wait_dequeue(msg_);
    return msg_;
}

auto client::read_wait(std::chrono::milliseconds ms) -> std::optional<message> {
    message msg_;
    bool status = read_messages_.wait_dequeue_timed(msg_, ms);
    if (status)
        return msg_;
    return std::nullopt;
}

auto client::read_avail() -> size_t {
    return read_messages_.size_approx();
}

auto client::read_if_avail() -> std::optional<message> {
    message mes;
    if (read_messages_.try_dequeue(mes)) {
        return mes;
    }
    return std::nullopt;
}

void client::write(const message& msg) {
    auto lbd = [this, msg]() {
        bool write_in_progress = !write_msgs_.empty();
        write_msgs_.emplace_back(msg);
        if (!write_in_progress) {
            do_write_();
        }
    };
    boost::asio::post(io_context_, lbd);
}

void client::close() {
    boost::asio::post(io_context_, [this]() { socket_.close(); });
}

void client::do_connect_(const tcp::resolver::results_type& endpoints) {
    auto lbd = [this](boost::system::error_code ec, tcp::endpoint) {
        if (!ec) {
            do_read_header_();
        }
    };
    boost::asio::async_connect(socket_, endpoints, lbd);
}

void client::do_read_header_() {
    auto buf = boost::asio::buffer(read_msg_.data(), message::header_length);
    auto lbd = [this](boost::system::error_code ec, std::size_t /*length*/) {
        if (!ec && read_msg_.decode_header()) {
            do_read_body_();
        } else {
            socket_.close();
        }
    };
    boost::asio::async_read(socket_, buf, lbd);
}

void client::do_read_body_() {
    auto buf = boost::asio::buffer(read_msg_.body(), read_msg_.body_length());
    auto lbd = [this](boost::system::error_code ec, std::size_t /*length*/) {
        if (!ec) {
            read_messages_.enqueue(read_msg_);
            do_read_header_();
        } else {
            socket_.close();
        }
    };
    boost::asio::async_read(socket_, buf, lbd);
}

void client::do_write_() {
    auto buf = boost::asio::buffer(write_msgs_.front().data(),
                                   write_msgs_.front().length());
    auto lbd = [this](boost::system::error_code ec, std::size_t /*length*/) {
        if (!ec) {
            write_msgs_.pop_front();
            if (!write_msgs_.empty()) {
                do_write_();
            }
        } else {
            socket_.close();
        }
    };
    boost::asio::async_write(socket_, buf, lbd);
}
