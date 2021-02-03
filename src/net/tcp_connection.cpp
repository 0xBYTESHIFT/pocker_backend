#include "net/tcp_connection.h"
#include "api.h"
#include "components/log.hpp"
#include "net/mes_handler.h"

auto tcp_connection::create(io_context_t& io_context) -> pointer_t {
    return pointer_t(new tcp_connection(io_context));
}

auto tcp_connection::socket() -> socket_t& {
    return socket_;
}

void tcp_connection::start() {
    do_read_header_();
}

tcp_connection::tcp_connection(io_context_t& io_context)
    : socket_(io_context) {}

void tcp_connection::add_handler(std::shared_ptr<mes_handler> handler) {
    this->handler_ = handler;
}

void tcp_connection::deliver(const message& msg) {
    bool write_in_progress = !write_msgs_.empty();
    write_msgs_.push_back(msg);
    if (!write_in_progress) {
        do_write_();
    }
}

void tcp_connection::do_read_header_() {
    auto self(shared_from_this());
    auto lbd = [this, self](boost::system::error_code ec, std::size_t /*length*/) {
        auto lgr = get_logger();
        static const auto prefix = "tcp_connection::do_read_header_";
        if (!ec) {
            if (read_msg_.decode_header()) {
                do_read_body_();
            } else {
                lgr.error("{} unable to decode message header", prefix);
            }
        } else {
            lgr.error("{} recv error:{}", prefix, ec.message());
            handler_->del(shared_from_this());
        }
    };
    auto buf = boost::asio::buffer(read_msg_.data(), message::header_length);
    boost::asio::async_read(socket_, buf, lbd);
}

void tcp_connection::do_read_body_() {
    auto self(shared_from_this());
    auto buf = boost::asio::buffer(read_msg_.body(), read_msg_.body_length());
    auto lbd = [this, self](boost::system::error_code ec, std::size_t /*length*/) {
        auto lgr = get_logger();
        static const auto prefix = "tcp_connection::do_read_body_";
        if (!ec) {
            auto mes = std::string_view(read_msg_.body(), read_msg_.body_length());
            lgr.debug("{} got message:{}", prefix, mes);
            handler_->broadcast(read_msg_, self);
            do_read_header_();
        } else {
            lgr.error("{} recv error:{}", prefix, ec.message());
            handler_->del(shared_from_this());
        }
    };
    boost::asio::async_read(socket_, buf, lbd);
}

void tcp_connection::do_write_() {
    auto self(shared_from_this());
    auto buf = boost::asio::buffer(write_msgs_.front().data(), write_msgs_.front().length());
    auto lbd = [this, self](boost::system::error_code ec, std::size_t /*length*/) {
        if (!ec) {
            write_msgs_.pop_front();
            if (!write_msgs_.empty()) {
                do_write_();
            }
        } else {
            auto lgr = get_logger();
            static const auto prefix = "tcp_connection::do_write_";
            lgr.error("{} send error:{}", prefix, ec.message());
            handler_->del(shared_from_this());
        }
    };
    boost::asio::async_write(socket_, buf, lbd);
}