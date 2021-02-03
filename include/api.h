#pragma once
#include "components/property.hpp"
#include "rapidjson/prettywriter.h"
#include <string>
#include <codecvt>

namespace api {

    template<class T>
    struct val_t {
        val_t() = delete;

        val_t(const std::string& name, const T& def_val) {
            this->name_ = name;
            this->val_ = def_val;
        }

        auto operator=(const T& arg) -> val_t& {
            this->val_ = arg;
            return *this;
        }

        operator T() {
            return val_;
        }

        operator T() const {
            return val_;
        }

        auto name() const -> const std::string& {
            return name_;
        }

    protected:
        std::string name_ = "";
        T val_;
    };

    template<class T>
    using prop_val = property<val_t<T>>;

    template<class Writer>
    void write_str(const std::string& str, Writer& w) {
        auto mes_size = static_cast<rapidjson::SizeType>(str.length());
        w.String(str.c_str(), mes_size);
    }

    template<class Writer>
    void write_str(const std::wstring& str, Writer& w) {
        auto mes_size = static_cast<rapidjson::SizeType>(str.length()*sizeof(wchar_t));
        w.String(reinterpret_cast<const char*>(str.c_str()), mes_size);
    }

    struct connect_response {
        const prop_val<std::string> type{"type", "connect_response"};
        prop_val<size_t> code{"code", 0};

        void from_json(const std::string &str);
        auto to_json() const -> std::string;
    };

    struct register_request {
        const prop_val<std::string> type{"type", "register_request"};
        prop_val<std::wstring> first_name = {"first_name", L""};
        prop_val<std::wstring> last_name = {"last_name", L""};
        prop_val<std::string> pass_hash = {"pass_hash", ""};

        void from_json(const std::string& json);
        auto to_json() const -> std::string;
    };

    struct register_response {
        const property<std::string> type = "register_response";
        enum class code {
            OK,
            NAME_TAKEN,
            ETC
        };
        property<size_t> code = 0;

        void from_json(const std::string &jspn);
        auto to_json() const -> std::string;
    };

    struct login_request {
        const property<std::string> type = "login_request";
        property<std::string> name = "";
        property<std::string> pass_hash = "";

        void from_json();
        auto to_json() const -> std::string;
    };

    struct login_response {
        const property<std::string> type = "login_response";
        enum class code {
            OK,
            NAME_UNKNOWN,
            PASS_INCORRECT,
            ETC
        };
        property<code> code;

        void from_json();
        auto to_json() const -> std::string;
    };
}; // namespace api