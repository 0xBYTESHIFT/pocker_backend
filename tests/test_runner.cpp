#include "log.hpp"

#include <boost/process.hpp>
#include <boost/program_options.hpp>
#include <boost/stacktrace.hpp>
#include <csignal>
#include <iostream>
#include <sstream>

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

int main(int argc, char* argv[]) {
    namespace bp = boost::process;

    setup_handlers();
    std::set_terminate(terminate_handler);

    auto lgr = initialization_logger();
    lgr.set_level(logger::level::info);
    auto internal = lgr.get_internal_logger();
    internal->set_pattern("[%Y-%m-%d %T] [%L] %v");

    //parse options
    namespace po = boost::program_options;
    po::options_description desc("Allowed options");
    desc.add_options()("serv_path", po::value<std::string>(), "path of server exec");
    desc.add_options()("test_path", po::value<std::string>(), "path of test cli exec");
    desc.add_options()("port", po::value<std::string>(), "accepting port for server");
    desc.add_options()("db_config", po::value<std::string>(), "db json configuration file path");
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    std::string port;
    std::string db_conf_path, serv_path, test_path;

    if (vm.count("port") > 0) {
        port = vm["port"].as<std::string>();
    } else {
        lgr.critical("'port' parameter is mandatory!");
        abort();
    }
    if (vm.count("db_config") > 0) {
        db_conf_path = vm["db_config"].as<std::string>();
    } else {
        lgr.critical("'db_config' parameter is mandatory!");
        abort();
    }
    if (vm.count("serv_path") > 0) {
        serv_path = vm["serv_path"].as<std::string>();
    } else {
        lgr.critical("'serv_path' parameter is mandatory!");
        abort();
    }
    if (vm.count("test_path") > 0) {
        test_path = vm["test_path"].as<std::string>();
    } else {
        lgr.critical("'test_path' parameter is mandatory!");
        abort();
    }

    bp::child serv(serv_path, "--port", port, "--db_config", db_conf_path);
    lgr.info("server run");
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    bp::child cli(test_path, "--ip", "127.0.0.1", "--port", port);
    lgr.info("cli run, waiting");
    cli.wait();
    lgr.info("cli joining");
    cli.join();

    lgr.info("serv terminating");
    serv.terminate();

    if (cli.exit_code() != 0) {
        auto mes = fmt::format("cli exit code == {}", cli.exit_code());
        lgr.critical(mes);
        throw std::runtime_error(mes);
    }
}