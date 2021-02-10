#pragma once
#include <memory>
#include <sqlpp11/postgresql/connection.h>
#include <sqlpp11/sqlpp11.h>
#include <string>

class database_worker {
public:
    using connection_ptr = std::unique_ptr<sqlpp::postgresql::connection>;

    struct db_connect_config {
        std::string db_host;
        std::string db_name;
        std::string db_user;
        std::string db_pass;
        std::string db_pass_salt;
    };

    database_worker(const db_connect_config& config);
    ~database_worker();

    static auto read_cfg_from(const std::string& json_path) -> db_connect_config;
    void connect();
    auto db() -> connection_ptr&;
    auto config() const -> const db_connect_config&;

private:
    std::shared_ptr<sqlpp::postgresql::connection_config> sqlpp_config_;
    connection_ptr db_connection_;
    db_connect_config config_;
};