#include "components/api/api.h"
#include "components/client/client.h"
#include "components/log.hpp"
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

static auto hash(const std::string& orig) -> std::string {
    std::string result(SHA512_DIGEST_LENGTH, ' ');
    SHA512(reinterpret_cast<const unsigned char*>(orig.data()), orig.size(),
           reinterpret_cast<unsigned char*>(result.data()));
    return boost::algorithm::hex(result);
}

void send_reg_request(client& c, const std::string& line) {
    std::vector<std::string> words;
    boost::split(words, line, boost::is_any_of("\t;"));

    api::register_request rq;
    rq.nickname = words.at(1);
    rq.email = words.at(2);
    rq.pass_hash = hash(words.at(3));

    message msg = rq.to_json();
    c.write(msg);
}

void send_login_request(client& c, const std::string& line) {
    std::vector<std::string> words;
    boost::split(words, line, boost::is_any_of("\t;"));

    api::login_request rq;
    rq.email = words.at(1);
    rq.pass_hash = hash(words.at(2));

    message msg = rq.to_json();
    c.write(msg);
}

void try_register(client& c) {
    auto lgr = get_logger();
    auto prefix = "try_register";

    message msg;
    api::register_request reg_req;
    reg_req.email = "some_test_email@mail.domain";
    reg_req.nickname = "nick";
    reg_req.pass_hash = hash("some_test_hash");
    msg = reg_req.to_json();
    c.write(msg);

    msg = c.read_wait();
    std::string msg_str(msg.body(), msg.body_length());
    lgr.info("{} got mes:{}", prefix, msg_str);

    json j;
    j.parse(msg_str);
    if (j.value_as<std::string>("type") ==
        api::register_response::type().downcast()) {
        api::register_response resp;
        resp.from_json(j);
        if (resp.code() != api::register_response::code_enum::OK) {
            auto mes = fmt::format("{} code is not ok during first register");
            throw std::runtime_error(mes);
        }
    } else {
        lgr.critical("{} got something that is not a register response", prefix);
        abort();
    }
}

void try_re_register(client& c) {
    auto lgr = get_logger();
    auto prefix = "try_re_register";

    message msg;
    api::register_request reg_req;
    reg_req.email = "some_test_email@mail.domain";
    reg_req.nickname = "nick";
    reg_req.pass_hash = hash("some_test_hash");
    msg = reg_req.to_json();
    c.write(msg);

    msg = c.read_wait();
    std::string msg_str(msg.body(), msg.body_length());
    lgr.info("{} got mes:{}", prefix, msg_str);

    json j;
    j.parse(msg_str);
    if (j.value_as<std::string>("type") ==
        api::register_response::type().downcast()) {
        api::register_response resp;
        resp.from_json(j);
        if (resp.code() != api::register_response::code_enum::NAME_TAKEN) {
            auto mes = fmt::format("{} code is name_taken during second register");
            throw std::runtime_error(mes);
        }
    } else {
        lgr.critical("{} got something that is not a register response", prefix);
        abort();
    }
}

void try_un_register(client& c) {
    auto lgr = get_logger();
    auto prefix = "try_un_register";

    message msg;
    api::unregister_request unreg_req;
    unreg_req.email = "some_test_email@mail.domain";
    unreg_req.pass_hash = hash("some_test_hash");
    msg = unreg_req.to_json();
    c.write(msg);

    //get delete register response
    msg = c.read_wait();
    std::string msg_str(msg.body(), msg.body_length());
    lgr.info("{} got mes:{}", prefix, msg_str);
    json j;
    j.parse(msg_str);
    if (j.value_as<std::string>("type") ==
        api::unregister_response::type().downcast()) {
        api::unregister_response resp;
        resp.from_json(j);
        if (resp.code() != api::unregister_response::code_enum::OK) {
            auto mes = fmt::format("{} wrong code on unreq request:{}", prefix, (int) resp.code().downcast());
            throw std::runtime_error(mes);
        }
    } else {
        lgr.critical("{} got something that is not an unregister response", prefix);
        abort();
    }
}

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

    c.read_wait();//skip connection response
    try_register(c);
    try_re_register(c);
    try_un_register(c);

    c.close();
    t.join();

    return 0;
}