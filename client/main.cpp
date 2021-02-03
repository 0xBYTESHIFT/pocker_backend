#include "components/log.hpp"
#include "net/message.hpp"

#include <boost/program_options.hpp>
#include <boost/stacktrace.hpp>
#include <csignal>
#include <iostream>

#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <iostream>
#include <deque>

using boost::asio::ip::tcp;

void my_signal_handler(int signum) {
    ::signal(signum, SIG_DFL);
    std::cerr << "signal called:" << std::endl
              << boost::stacktrace::stacktrace() << std::endl;
    std::cerr << "errno:" << ::strerror(errno) << std::endl;
    ::raise(SIGABRT);
}

void setup_handlers() {
    ::signal(SIGSEGV, &my_signal_handler);
    ::signal(SIGABRT, &my_signal_handler);
}

static auto original_terminate_handler{std::get_terminate()};

void terminate_handler() {
    std::cerr << "terminate called:" << std::endl
              << boost::stacktrace::stacktrace() << std::endl;
    std::cerr << "errno:" << ::strerror(errno) << std::endl;
    original_terminate_handler();
    std::abort();
}

using boost::asio::ip::tcp;

using message_queue = std::deque<message>;

class client {
public:
    using io_context_t = boost::asio::io_context;

    client(io_context_t& io_context,
           const tcp::resolver::results_type& endpoints)
        : io_context_(io_context)
        , socket_(io_context) {
        do_connect_(endpoints);
    }

    void write(const message& msg) {
        auto lbd = [this, msg]() {
            bool write_in_progress = !write_msgs_.empty();
            write_msgs_.emplace_back(msg);
            if (!write_in_progress) {
                do_write_();
            }
        };
        boost::asio::post(io_context_, lbd);
    }

    void close() {
        boost::asio::post(io_context_, [this]() { socket_.close(); });
    }

private:
    void do_connect_(const tcp::resolver::results_type& endpoints) {
        auto lbd = [this](boost::system::error_code ec, tcp::endpoint) {
            if (!ec) {
                do_read_header_();
            }
        };
        boost::asio::async_connect(socket_, endpoints, lbd);
    }

    void do_read_header_() {
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

    void do_read_body_() {
        auto buf = boost::asio::buffer(read_msg_.body(), read_msg_.body_length());
        auto lbd = [this](boost::system::error_code ec, std::size_t /*length*/) {
            if (!ec) {
                std::cout.write(read_msg_.body(), read_msg_.body_length());
                std::cout << "\n";
                do_read_header_();
            } else {
                socket_.close();
            }
        };
        boost::asio::async_read(socket_, buf, lbd);
    }

    void do_write_() {
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

private:
    boost::asio::io_context& io_context_;
    tcp::socket socket_;
    message read_msg_;
    message_queue write_msgs_;
};

int main(int argc, char* argv[]) {
    setup_handlers();
    std::set_terminate(terminate_handler);

    auto lgr = initialization_logger();
    lgr.set_level(logger::level::info);
    auto internal = lgr.get_internal_logger();
    internal->set_pattern("[%Y-%m-%d %T] [%L] %v");

    //parse options
    namespace po = boost::program_options;
    po::options_description desc("Allowed options");
    desc.add_options()("help", "this message");
    desc.add_options()("ip", po::value<std::string>(), "ip of the server");
    desc.add_options()("port", po::value<std::string>(), "accepting port for server");
    desc.add_options()("verbose", po::value<size_t>(),
                       "level of verbosity.\n"
                       "0 - trace\n"
                       "1 - debug\n"
                       "2 - info\n"
                       "3 - warn\n"
                       "4 - err\n"
                       "5 - critical\n"
                       "6 - off\n");
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    if (vm.count("help") > 0) {
        std::stringstream ss;
        ss << desc;
        std::string mes = ss.str();
        lgr.info(mes);
        return 0;
    }
    if (vm.count("verbose") > 0) {
        int verb = vm["verbose"].as<size_t>();
        int max = (int) logger::level::level_nums;
        if (verb >= max) {
            auto mes = fmt::format("max alloved verbocity level:{}", max);
            lgr.error(mes);
            throw std::runtime_error(mes);
        }
        lgr.set_level(static_cast<logger::level>(verb));
    }
    std::string ip, port;
    if (vm.count("port") > 0) {
        port = vm["port"].as<std::string>();
    } else {
        lgr.critical("'port' parameter is mandatory!");
        abort();
    }
    if (vm.count("ip") > 0) {
        ip = vm["ip"].as<std::string>();
    } else {
        lgr.critical("'ip' parameter is mandatory!");
        abort();
    }

    boost::asio::io_context io_context;
    tcp::resolver resolver(io_context);
    auto endpoints = resolver.resolve(ip, port);
    client c(io_context, endpoints);

    std::thread t([&io_context]() { io_context.run(); });

    char line[message::max_body_length + 1];
    while (std::cin.getline(line, message::max_body_length + 1)) {
        message msg;
        msg.body_length(std::strlen(line));
        std::memcpy(msg.body(), line, msg.body_length());
        msg.encode_header();
        c.write(msg);
    }

    c.close();
    t.join();

    return 0;
}