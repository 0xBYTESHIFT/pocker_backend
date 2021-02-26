#include "net/tcp_server.h"
#include "components/api/api.h"
#include "components/log.hpp"
#include "tracy_include.h"
#include <boost/bind.hpp>

tcp_server::tcp_server(io_context_t& io_context, size_t port)
    : io_context_(io_context)
    , acceptor_(io_context, tcp::endpoint(tcp::v4(), port)) {
    ZoneScoped;
    start_accept();
    this->handler_ = std::make_shared<mes_handler>();
}

void tcp_server::start_accept() {
    ZoneScoped;
    auto lgr = get_logger();
    auto prefix = "tcp_server::start_accept";
    lgr.debug("{} waiting for connection", prefix);
    auto new_connection = tcp_connection::create(io_context_);

    auto func = boost::bind(&tcp_server::handle_accept, this, new_connection, boost::asio::placeholders::error);
    acceptor_.async_accept(new_connection->socket(), func);
}

void tcp_server::handle_accept(pointer_t new_connection, const ec_t& error) {
    ZoneScoped;
    auto lgr = get_logger();
    auto prefix = "tcp_server::handle_accept";
    api::connect_response resp;
    resp.code = 0; //ok;
    message msg = resp.to_json();

    if (!error) {
        lgr.debug("{} new connection accepted", prefix);
        new_connection->deliver(msg);
        handler_->add_connection(new_connection);
    } else {
        lgr.error("{} error:{}", prefix, error.message());
    }

    start_accept();
}

auto tcp_server::get_mes_handler() const -> std::shared_ptr<mes_handler> {
    ZoneScoped;
    return this->handler_;
}