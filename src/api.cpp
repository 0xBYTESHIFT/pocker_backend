#include "include/api.h"
#include "rapidjson/document.h"
#include "rapidjson/prettywriter.h"
#include <locale>
#include <codecvt>

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
    rapidjson::Document d;
    d.Parse(json.c_str());
    rapidjson::Value val;

    val = d[this->first_name().name().c_str()];
    this->first_name() = std::wstring_convert<std::codecvt_utf16<wchar_t>>().from_bytes(val.GetString());

    val = d[this->last_name().name().c_str()];
    this->last_name() = std::wstring_convert<std::codecvt_utf16<wchar_t>>().from_bytes(val.GetString());

    val = d[this->pass_hash().name().c_str()];
    this->pass_hash() = val.GetString();
}

auto register_request::to_json() const -> std::string {
    std::string result;
    rapidjson::StringBuffer sb;
    rapidjson::PrettyWriter<decltype(sb)> pw(sb);

    pw.StartObject();
    api::write_str(this->type().name(), pw);
    api::write_str(this->type(), pw);

    api::write_str(this->first_name().name(), pw);
    api::write_str(this->first_name(), pw);

    api::write_str(this->last_name().name(), pw);
    api::write_str(this->last_name(), pw);

    api::write_str(this->pass_hash().name(), pw);
    api::write_str(this->pass_hash(), pw);
    pw.EndObject();

    result = sb.GetString();
    return result;
}

void register_response::from_json(const std::string& json) {
}
auto register_response::to_json() const -> std::string {
    std::string result;
    return result;
}