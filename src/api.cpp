#include "include/api.h"
#include "rapidjson/document.h"
#include "rapidjson/prettywriter.h"
#include "tracy_include.h"
#include <codecvt>
#include <locale>

using namespace api;

template<class Pw>
void write_prop_val(Pw& pw, const prop_val<std::string>& val) {
    ZoneScoped;
    api::write_str(val().name(), pw);
    api::write_str(val(), pw);
}
template<class Pw>
void write_prop_val(Pw& pw, const prop_val<size_t>& val) {
    ZoneScoped;
    api::write_str(val().name(), pw);
    pw.Int64(val().downcast());
}

void connect_response::from_json(const std::string& str) {
    ZoneScoped;
    rapidjson::Document d;
    d.Parse(str.c_str());
    rapidjson::Value val;
    val = d[this->code().name().c_str()];
    this->code() = val.GetInt64();
}
auto connect_response::to_json() const -> std::string {
    ZoneScoped;
    std::string result;
    rapidjson::StringBuffer sb;
    rapidjson::PrettyWriter<decltype(sb)> pw(sb);

    pw.StartObject();
    write_prop_val(pw, this->type);
    write_prop_val(pw, this->code);
    pw.EndObject();

    result = sb.GetString();
    return result;
}
void register_request::from_json(const std::string& json) {
    ZoneScoped;
    class json j = json;
    this->from_json(j);
}

/*==============================================================*/
void register_request::from_json(const json& j) {
    ZoneScoped;
    this->nickname() = j.value_as<std::string>(this->nickname().name());
    this->email() = j.value_as<std::string>(this->email().name());
    this->pass_hash() = j.value_as<std::string>(this->pass_hash().name());
}

auto register_request::to_json() const -> std::string {
    ZoneScoped;
    std::string result;
    rapidjson::StringBuffer sb;
    rapidjson::PrettyWriter<decltype(sb)> pw(sb);

    pw.StartObject();
    write_prop_val(pw, this->type);
    write_prop_val(pw, this->email);
    write_prop_val(pw, this->nickname);
    write_prop_val(pw, this->pass_hash);
    pw.EndObject();

    result = sb.GetString();
    return result;
}

/*==============================================================*/
void register_response::from_json(const std::string& json) {
    ZoneScoped;
    class json j = json;
    this->from_json(j);
}
void register_response::from_json(const json& j) {
    ZoneScoped;
    this->message() = j.value_as<std::string>(this->message().name());
    this->code() = (register_response::code_enum) j.value_as<std::size_t>(this->code().name());
}
auto register_response::to_json() const -> std::string {
    ZoneScoped;
    std::string result;
    rapidjson::StringBuffer sb;
    rapidjson::PrettyWriter<decltype(sb)> pw(sb);

    pw.StartObject();
    write_prop_val(pw, this->type);
    write_prop_val(pw, this->message);

    api::write_str(this->code().name(), pw);
    pw.Int64(size_t(this->code().downcast()));
    pw.EndObject();

    result = sb.GetString();
    return result;
}

/*==============================================================*/
void login_request::from_json(const std::string& json) {
    ZoneScoped;
    class json j = json;
    this->from_json(j);
}
void login_request::from_json(const json& j) {
    ZoneScoped;
    this->email() = j.value_as<std::string>(this->email().name());
    this->pass_hash() = j.value_as<std::string>(this->pass_hash().name());
}
auto login_request::to_json() const -> std::string {
    ZoneScoped;
    std::string result;
    rapidjson::StringBuffer sb;
    rapidjson::PrettyWriter<decltype(sb)> pw(sb);

    pw.StartObject();
    write_prop_val(pw, this->type);
    write_prop_val(pw, this->email);
    write_prop_val(pw, this->pass_hash);
    pw.EndObject();

    result = sb.GetString();
    return result;
}

/*==============================================================*/
void login_response::from_json(const std::string& json) {
    ZoneScoped;
    class json j = json;
    this->from_json(j);
}
void login_response::from_json(const json& j) {
    ZoneScoped;
    this->code() = (login_response::code_enum) j.value_as<std::size_t>(this->code().name());
    this->message() = j.value_as<std::string>(this->message().name());
}
auto login_response::to_json() const -> std::string {
    ZoneScoped;
    std::string result;
    rapidjson::StringBuffer sb;
    rapidjson::PrettyWriter<decltype(sb)> pw(sb);

    pw.StartObject();
    write_prop_val(pw, this->type);
    write_prop_val(pw, this->message);

    api::write_str(this->code().name(), pw);
    pw.Int64(size_t(this->code().downcast()));
    pw.EndObject();

    result = sb.GetString();
    return result;
}