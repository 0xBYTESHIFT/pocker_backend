#pragma once
#include "net/tcp_connection.h"
#include "property.hpp"
#include "token.hpp"
#include <string>

class user {
public:
    property<std::string> nickname;
    property<std::string> email;
    property<size_t> id;
    property<class token> token;
    property<tcp_connection_ptr> connection;

    user(size_t id, const tcp_connection_ptr& conn);
};