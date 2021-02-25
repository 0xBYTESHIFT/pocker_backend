#pragma once
#include "tracy_include.h"
#include <codecvt>
#include <cstring>
#include <string>

class message {
public:
    enum { header_length = 4 };
    enum { max_body_length = 2048 - header_length };

    message() {
        ZoneScoped;
    }

    message(const std::string& str) {
        ZoneScoped;
        (*this) = str;
    }

    message(const std::wstring& str) {
        ZoneScoped;
        (*this) = str;
    }

    const char* data() const {
        ZoneScoped;
        return data_;
    }

    char* data() {
        ZoneScoped;
        return data_;
    }

    std::size_t length() const {
        ZoneScoped;
        return header_length + body_length_;
    }

    const char* body() const {
        ZoneScoped;
        return data_ + header_length;
    }

    char* body() {
        ZoneScoped;
        return data_ + header_length;
    }

    std::size_t body_length() const {
        ZoneScoped;
        return body_length_;
    }

    void body_length(std::size_t new_length) {
        ZoneScoped;
        body_length_ = new_length;
        if (body_length_ > max_body_length)
            body_length_ = max_body_length;
    }

    bool decode_header() {
        ZoneScoped;
        char header[header_length + 1] = "";
        std::strncat(header, data_, header_length);
        body_length_ = std::atoi(header);
        if (body_length_ > max_body_length) {
            body_length_ = 0;
            return false;
        }
        return true;
    }

    void encode_header() {
        ZoneScoped;
        char header[header_length + 1] = "";
        std::sprintf(header, "%4d", static_cast<int>(body_length_));
        std::memcpy(data_, header, header_length);
    }

    auto operator=(const std::string& str) -> message& {
        ZoneScoped;
        const auto len = str.length();
        body_length(len);
        encode_header();
        std::memcpy(data_ + header_length, str.data(), len);
        return *this;
    }

    auto operator=(const std::wstring& str) -> message& {
        ZoneScoped;
        const auto len = str.length() * sizeof(wchar_t);
        body_length(len);
        encode_header();
        std::memcpy(data_ + header_length, str.data(), len);
        return *this;
    }

private:
    char data_[header_length + max_body_length];
    std::size_t body_length_ = 0;
};