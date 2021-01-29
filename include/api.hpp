#pragma once
#include "components/property.hpp"
#include <string>

namespace api {

    template<class Size, class Str, class Writer>
    void write_str(const Str& str, Writer& w) {
        auto mes_size = static_cast<Size>(str.length());
        w.String(str.c_str(), mes_size);
    }

    struct connect_response {
        property<size_t> code = 0;
        property<std::string> message = "";
        property<std::string> type = "connect_response";

        void from_json();
        auto to_json() const -> std::string;
    };
}; // namespace api