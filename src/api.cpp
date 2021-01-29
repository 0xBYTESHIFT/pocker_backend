#include "include/api.hpp"
#include "rapidjson/prettywriter.h"

using namespace api;

void connect_response::from_json() {
}
auto connect_response::to_json() const -> std::string {
    std::string result;
    rapidjson::StringBuffer sb;
    rapidjson::PrettyWriter<decltype(sb)> pw(sb);
    pw.StartObject();
    pw.String("type");
    api::write_str<rapidjson::SizeType>(this->type(), pw);

    pw.String("code");
    pw.Int64(this->code());

    pw.String("message");
    api::write_str<rapidjson::SizeType>(this->message(), pw);
    pw.EndObject();

    result = sb.GetString();
    return result;
}