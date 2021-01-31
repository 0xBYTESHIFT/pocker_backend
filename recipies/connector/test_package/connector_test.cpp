#include <sqlpp11/postgresql/connection.h>
#include <sqlpp11/postgresql/exception.h>
#include <sqlpp11/sqlpp11.h>

namespace sql = sqlpp::postgresql;
int main(int, char*[]) {
    auto config = std::make_shared<sql::connection_config>();
    config->host = "localhost";
    config->user = "unknown_user_must_fail";
    return 0;
}