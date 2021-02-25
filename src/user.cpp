#include "user.h"
#include "tracy_include.h"

user::user(size_t id, const tcp_connection_ptr& conn) {
    ZoneScoped;
    this->id = id;
    this->connection = conn;
}