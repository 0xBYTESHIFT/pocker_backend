#pragma once
#include <string>
#include "property.hpp"
#include "net/tcp_connection.h"

class user{
public:
    property<std::string> nickname;
    property<std::string> email;
    property<size_t> id;

    property<tcp_connection_ptr> connection;

    user(size_t id, const tcp_connection_ptr &conn);
};