#include "include/api.h"
#include "rapidjson/document.h"
#include "rapidjson/prettywriter.h"

using namespace api;

void connect_response::from_json(const std::string& str) {
    rapidjson::Document d;
    d.Parse(str.c_str());
    rapidjson::Value val;
    val = d[this->code().name().c_str()];
    this->code() = val.GetInt64();
}

auto connect_response::to_json() const -> std::string {
    std::string result;
    rapidjson::StringBuffer sb;
    rapidjson::PrettyWriter<decltype(sb)> pw(sb);

    pw.StartObject();
    api::write_str(this->type().name(), pw);
    api::write_str(this->type(), pw);

    api::write_str(this->code().name(), pw);
    pw.Int64(this->code());
    pw.EndObject();

    result = sb.GetString();
    return result;
}

void register_request::from_json(const std::string& json) {
}
auto register_request::to_json() const -> std::string {
    std::string result;
    return result;
}

void register_response::from_json(const std::string& json) {
}
auto register_response::to_json() const -> std::string {
    std::string result;
    return result;
}