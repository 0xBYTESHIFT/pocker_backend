#include <boost/program_options.hpp>
#include <boost/stacktrace.hpp>
#include <csignal>
#include <fstream>
#include <iostream>

#include "json_obj.h"
#include "log.hpp"

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
    desc.add_options()("db_config", po::value<std::string>(), "");
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    std::string db_config_path;
    if (vm.count("db_config") > 0) {
        db_config_path = vm["db_config"].as<std::string>();
    } else {
        lgr.critical("'db_config' parameter is mandatory!");
        abort();
    }

    std::string content;
    {
        std::ifstream ifs(db_config_path);
        if (!ifs.good()) {
            lgr.error("file error at {}: {}", db_config_path, strerror(errno));
            abort();
        }
        ifs.seekg(0, ifs.end);
        size_t size = ifs.tellg();
        ifs.seekg(0, ifs.beg);

        content.reserve(size);
        content.assign(std::istreambuf_iterator<char>{ifs}, {});
    }
    json config(content);

    return 0;
}