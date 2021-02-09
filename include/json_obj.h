#pragma once
#include "rapidjson/document.h"
#include "rapidjson/prettywriter.h"
#include <string>

class json {
    rapidjson::Document d_;

public:
    struct error_code {
        enum class code_t {
            error_none = rapidjson::ParseErrorCode::kParseErrorNone,                                                 //!< No error.
            document_empty = rapidjson::ParseErrorCode::kParseErrorDocumentEmpty,                                    //!< The document is empty.
            document_root_not_singular = rapidjson::ParseErrorCode::kParseErrorDocumentRootNotSingular,              //!< The document root must not follow by other values.
            value_invalid = rapidjson::ParseErrorCode::kParseErrorValueInvalid,                                      //!< Invalid value.
            object_miss_name = rapidjson::ParseErrorCode::kParseErrorObjectMissName,                                 //!< Missing a name for object member.
            object_miss_colon = rapidjson::ParseErrorCode::kParseErrorObjectMissColon,                               //!< Missing a colon after a name of object member.
            object_miss_comma_or_bracket = rapidjson::ParseErrorCode::kParseErrorObjectMissCommaOrCurlyBracket,      //!< Missing a comma or '}' after an object member.
            array_miss_comma_or_bracket = rapidjson::ParseErrorCode::kParseErrorArrayMissCommaOrSquareBracket,       //!< Missing a comma or ']' after an array element.
            string_unicode_escape_invalid_hex = rapidjson::ParseErrorCode::kParseErrorStringUnicodeEscapeInvalidHex, //!< Incorrect hex digit after \\u escape in string.
            string_unicode_surrogate_invalid = rapidjson::ParseErrorCode::kParseErrorStringUnicodeSurrogateInvalid,  //!< The surrogate pair in string is invalid.
            string_escape_invalid = rapidjson::ParseErrorCode::kParseErrorStringEscapeInvalid,                       //!< Invalid escape character in string.
            string_miss_quotation_mark = rapidjson::ParseErrorCode::kParseErrorStringMissQuotationMark,              //!< Missing a closing quotation mark in string.
            string_invalid_encoding = rapidjson::ParseErrorCode::kParseErrorStringInvalidEncoding,                   //!< Invalid encoding in string.
            number_too_big = rapidjson::ParseErrorCode::kParseErrorNumberTooBig,                                     //!< Number too big to be stored in double.
            number_miss_fraction = rapidjson::ParseErrorCode::kParseErrorNumberMissFraction,                         //!< Miss fraction part in number.
            number_miss_exponent = rapidjson::ParseErrorCode::kParseErrorNumberMissExponent,                         //!< Miss exponent in number.
            error_termination = rapidjson::ParseErrorCode::kParseErrorTermination,                                   //!< Parsing was terminated.
            unspecific_syntax_error = rapidjson::ParseErrorCode::kParseErrorUnspecificSyntaxError,                   //!< Unspecific syntax error.
            value_type,                                                                                              //!< Attempt to get value with wrong type
            value_empty,                                                                                             //!< Value is empty
        };

        void reset();
        auto mes() const -> const std::string&;
        auto offset() const -> size_t;
        auto code() const -> code_t;

        operator bool() const;

    private:
        std::string mes_ = "Success";
        size_t offset_ = 0;
        code_t code_ = code_t::error_none;

        friend class json;
        void set_(code_t code, size_t offset);
    };

    using value_t = rapidjson::Value;
    using doc_t = rapidjson::Document;

    json() = default;
    ~json() = default;

    json(const std::string& json_str);

    json& operator=(const std::string& json_str);

    void parse(const std::string& json_str);
    void parse(const std::string& json_str, error_code& ec) noexcept;
    auto value(const std::string& name) -> value_t&;
    auto value(const std::string& name) const -> const value_t&;

    template<class T>
    auto value_as(const std::string& name) const -> T;

    template<class T>
    auto value_as(const std::string& name, error_code& ec) const noexcept -> T;

    auto inner_doc() -> doc_t&;
};

/*======*/
inline void json::error_code::reset() {
    this->code_ = code_t::error_none;
    this->mes_ = "success";
}

inline auto json::error_code::mes() const -> const std::string& {
    return this->mes_;
}

inline auto json::error_code::offset() const -> size_t {
    return this->offset_;
}

inline auto json::error_code::code() const -> code_t {
    return this->code_;
}

inline json::error_code::operator bool() const {
    return code_ != code_t::error_none;
}

inline void json::error_code::set_(code_t code, size_t offset) {
    this->code_ = code;
    this->offset_ = offset;

    using namespace rapidjson;

    switch ((int) code) {
        case ParseErrorCode::kParseErrorNone:
            mes_ = "Success";
            break;
        case ParseErrorCode::kParseErrorDocumentEmpty:
            mes_ = "The document is empty";
            break;
        case ParseErrorCode::kParseErrorDocumentRootNotSingular:
            mes_ = "The document root must not follow by other values";
            break;
        case ParseErrorCode::kParseErrorValueInvalid:
            mes_ = "Invalid value";
            break;
        case ParseErrorCode::kParseErrorObjectMissName:
            mes_ = "Missing a name for object member";
            break;
        case ParseErrorCode::kParseErrorObjectMissColon:
            mes_ = "Missing a colon after a name of object member";
            break;
        case ParseErrorCode::kParseErrorObjectMissCommaOrCurlyBracket:
            mes_ = "Missing a comma or '}' after an object member";
            break;
        case ParseErrorCode::kParseErrorArrayMissCommaOrSquareBracket:
            mes_ = "Missing a comma or ']' after an array element";
            break;
        case ParseErrorCode::kParseErrorStringUnicodeEscapeInvalidHex:
            mes_ = "Incorrect hex digit after \\u escape in string";
            break;
        case ParseErrorCode::kParseErrorStringUnicodeSurrogateInvalid:
            mes_ = "The surrogate pair in string is invalid";
            break;
        case ParseErrorCode::kParseErrorStringEscapeInvalid:
            mes_ = "Invalid escape character in string";
            break;
        case ParseErrorCode::kParseErrorStringMissQuotationMark:
            mes_ = "Missing a closing quotation mark in string";
            break;
        case ParseErrorCode::kParseErrorStringInvalidEncoding:
            mes_ = "Invalid encoding in string";
            break;
        case ParseErrorCode::kParseErrorNumberTooBig:
            mes_ = "Number too big to be stored in double";
            break;
        case ParseErrorCode::kParseErrorNumberMissFraction:
            mes_ = "Miss fraction part in number";
            break;
        case ParseErrorCode::kParseErrorNumberMissExponent:
            mes_ = "Miss exponent in number";
            break;
        case ParseErrorCode::kParseErrorTermination:
            mes_ = "Parsing was terminated";
            break;
        case ParseErrorCode::kParseErrorUnspecificSyntaxError:
            mes_ = "Unspecific syntax error";
            break;
        case (int) code_t::value_type:
            mes_ = "Attempt to get value with wrong type";
            break;
        case (int) code_t::value_empty:
            mes_ = "Value is empty";
            break;
        default:
            mes_ = "Unknown";
            break;
    };
}

/*======*/

inline json::json(const std::string& json_str) {
    this->parse(json_str);
}

inline json& json::operator=(const std::string& json_str) {
    this->parse(json_str);
    return *this;
}

inline void json::parse(const std::string& json_str) {
    error_code ec;
    this->parse(json_str, ec);
    if (ec) {
        std::string mes = ec.mes();
        mes += "\njson:" + json_str;
        throw std::runtime_error(mes);
    }
}

inline void json::parse(const std::string& json_str, error_code& ec) noexcept {
    d_.Parse(json_str.c_str());
    if (d_.HasParseError()) {
        ec.set_((error_code::code_t) d_.GetParseError(), d_.GetErrorOffset());
    }
}

inline auto json::value(const std::string& name) -> value_t& {
    auto& val = d_[name.c_str()];
    return val;
}

inline auto json::value(const std::string& name) const -> const value_t& {
    const auto& val = d_[name.c_str()];
    return val;
}

template<class T>
auto json::value_as(const std::string& name) const -> T {
    error_code ec;
    auto val = this->value_as<T>(name, ec);
    if (ec) {
        std::string mes = ec.mes();
        mes += "\njson value name:" + name;
        throw std::runtime_error(mes);
    }
    return val;
}

template<class T>
auto json::value_as(const std::string& name, error_code& ec) const noexcept -> T {
    auto& val = d_[name.c_str()];
    if (val.IsNull()) {
        ec.set_(error_code::code_t::value_empty, 0);
        return T();
    }
    if constexpr (std::is_same_v<T, std::string>) {
        if (!val.IsString()) {
            ec.set_(error_code::code_t::value_type, 0);
            return T();
        }
        std::string str(val.GetString(), val.GetStringLength());
        return str;
    } else {
        if (!val.Is<T>()) {
            ec.set_(error_code::code_t::value_type, 0);
            return T();
        }
        return val.Get<T>();
    }
}

inline auto json::inner_doc() -> doc_t& {
    return this->d_;
}