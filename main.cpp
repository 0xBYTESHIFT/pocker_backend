#include "components/log.hpp"

#include <boost/program_options.hpp>
#include <boost/stacktrace.hpp>
#include <csignal>
#include <deque>
#include <iostream>
#include <string>
#include <sqlpp11/sqlpp11.h>
#include <sqlpp11/postgresql/connection.h>

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

#include "net/tcp_connection.h"
#include "net/tcp_server.h"

auto main(int argc, char* argv[]) -> int {
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
    desc.add_options()("port", po::value<size_t>(), "accepting port for server");
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
    size_t port;
    if (vm.count("port") > 0) {
        port = vm["port"].as<size_t>();
    } else {
        lgr.critical("'port' parameter is mandatory!");
        abort();
    }

    boost::asio::io_context io_context;
    tcp_server server(io_context, port);
    io_context.run();

    return 0;
}
