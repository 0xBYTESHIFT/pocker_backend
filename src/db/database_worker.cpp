#include "db/database_worker.h"
#include "db/table_user.hpp"
#include "json_obj.h"
#include <fstream>
#include <string>

database_worker::database_worker(const db_connect_config& config) {
    this->config_ = config;
    this->sqlpp_config_ = std::make_shared<sqlpp::postgresql::connection_config>();
    sqlpp_config_->host = config_.db_host;
    sqlpp_config_->user = config_.db_user;
    sqlpp_config_->password = config_.db_pass;
    sqlpp_config_->dbname = config_.db_name;
}
database_worker::~database_worker() {}

void database_worker::connect() {
    this->db_connection_ = std::make_unique<sqlpp::postgresql::connection>(sqlpp_config_);
}

auto database_worker::db() -> connection_ptr& {
    return this->db_connection_;
}

auto database_worker::read_cfg_from(const std::string& json_path) -> db_connect_config {
    db_connect_config result;
    json j;
    std::ifstream ifs(json_path);
    std::string content;

    ifs.seekg(0, std::ios::end);
    content.reserve(ifs.tellg());
    ifs.seekg(0, std::ios::beg);

    content.assign((std::istreambuf_iterator<char>(ifs)),
                   std::istreambuf_iterator<char>());

    std::cout << "json:" << content << std::endl;
    j.parse(std::move(content));
    result.db_host = j.value_as<std::string>("db_host");
    result.db_name = j.value_as<std::string>("db_name");
    result.db_user = j.value_as<std::string>("db_user");
    result.db_pass = j.value_as<std::string>("db_pass");
    return result;
}