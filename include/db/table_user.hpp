#pragma once
#include <sqlpp11/char_sequence.h>
#include <sqlpp11/data_types.h>
#include <sqlpp11/table.h>

#define MAKE_MEMBER_T(name)                                \
    template<class T>                                      \
    struct _member_t {                                     \
        T name;                                            \
        T& operator()() { return this->name; }             \
        const T& operator()() const { return this->name; } \
    };

namespace tab_user_ {
    struct first_name {
        struct _alias_t {
            static constexpr const char _literal[] = "first_name";
            using _name_t = sqlpp::make_char_sequence<sizeof(_literal), _literal>;

            MAKE_MEMBER_T(first_name)
        };
        using _traits = sqlpp::make_traits<sqlpp::varchar>;
    };

    struct last_name {
        struct _alias_t {
            static constexpr const char _literal[] = "last_name";
            using _name_t = sqlpp::make_char_sequence<sizeof(_literal), _literal>;

            MAKE_MEMBER_T(last_name)
        };
        using _traits = sqlpp::make_traits<sqlpp::varchar>;
    };

    struct user_id {
        struct _alias_t {
            static constexpr const char _literal[] = "user_id";
            using _name_t = sqlpp::make_char_sequence<sizeof(_literal), _literal>;

            MAKE_MEMBER_T(user_id)
        };
        using _traits = sqlpp::make_traits<sqlpp::bigint>;
    };

    struct coins {
        struct _alias_t {
            static constexpr const char _literal[] = "coins";
            using _name_t = sqlpp::make_char_sequence<sizeof(_literal), _literal>;

            MAKE_MEMBER_T(coins)
        };
        using _traits = sqlpp::make_traits<sqlpp::bigint>;
    };
} // namespace tab_user_

struct tab_user : sqlpp::table_t<tab_user, tab_user_::first_name, tab_user_::last_name, tab_user_::user_id, tab_user_::coins> {
    struct _alias_t {
        static constexpr const char _literal[] = "users";
        using _name_t = sqlpp::make_char_sequence<sizeof(_literal), _literal>;

        template<class T>
        struct _member_t {
            T tab_user;
            T& operator()() { return tab_user; }
            const T& operator()() const { return tab_user; }
        };
    };
};