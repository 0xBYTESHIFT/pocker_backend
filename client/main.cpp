#include "api.h"
#include "components/log.hpp"
#include "net/client.h"
#include "net/message.hpp"

#include <boost/algorithm/hex.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/program_options.hpp>
#include <boost/stacktrace.hpp>
#include <codecvt>
#include <csignal>
#include <iostream>
#include <openssl/ssl3.h>

#include <boost/array.hpp>
#include <iostream>

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
    {
        message msg = c.read_wait();
        std::string msg_str(msg.body(), msg.body_length());
        std::cout << msg_str << std::endl;
    }

    std::wstring line;
    while (std::getline(std::wcin, line)) {
        if (line.find(L"/r") != std::wstring::npos) {
            std::vector<std::wstring> words;
            boost::split(words, line, boost::is_any_of("\t"));

            api::register_request rq;
            rq.first_name = words.at(1);
            if (words.at(2) != L"null") {
                rq.last_name = words.at(2);
            }

            auto& given_pass = words.at(3);
            std::string pass_hash(SHA512_DIGEST_LENGTH, ' ');
            std::cout << "doing sha512" << std::endl;
            SHA512(reinterpret_cast<unsigned char*>(given_pass.data()),
                   given_pass.size() * sizeof(wchar_t),
                   reinterpret_cast<unsigned char*>(pass_hash.data()));
            std::cout << "doing sha512 ended" << std::endl;

            rq.pass_hash = boost::algorithm::hex(pass_hash);

            auto json_str = rq.to_json();
            std::cout << "json:" << json_str << std::endl;
            message msg = json_str;
            c.write(msg);
            msg = c.read_wait();
            std::string msg_str(msg.body(), msg.body_length());
            std::cout << msg_str << std::endl;
            std::cout << "read done" << std::endl;
        } else {
            message msg = line;
            c.write(msg);
        }
    }

    c.close();
    t.join();

    return 0;
}