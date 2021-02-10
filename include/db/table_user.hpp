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

#define MAKE_LITERAL(name)                          \
    static constexpr const char _literal[] = #name; \
    using _name_t = sqlpp::make_char_sequence<sizeof(_literal), _literal>;

namespace tab_user_ {
    struct nickname {
        struct _alias_t {
            MAKE_LITERAL(nickname)
            MAKE_MEMBER_T(nickname)
        };
        using _traits = sqlpp::make_traits<sqlpp::varchar>;
    };

    struct user_id {
        struct _alias_t {
            MAKE_LITERAL(user_id)
            MAKE_MEMBER_T(user_id)
        };
        using _traits = sqlpp::make_traits<sqlpp::bigint>;
    };

    struct coins {
        struct _alias_t {
            MAKE_LITERAL(coins)
            MAKE_MEMBER_T(coins)
        };
        using _traits = sqlpp::make_traits<sqlpp::bigint>;
    };

    struct email {
        struct _alias_t {
            MAKE_LITERAL(email)
            MAKE_MEMBER_T(email)
        };
        using _traits = sqlpp::make_traits<sqlpp::varchar>;
    };

    struct pass_hash {
        struct _alias_t {
            MAKE_LITERAL(pass_hash)
            MAKE_MEMBER_T(pass_hash)
        };
        using _traits = sqlpp::make_traits<sqlpp::varchar>;
    };
} // namespace tab_user_

struct tab_user : sqlpp::table_t<tab_user, tab_user_::nickname, tab_user_::user_id, tab_user_::coins, tab_user_::email, tab_user_::pass_hash> {
    struct _alias_t {
        MAKE_LITERAL(users)
        MAKE_MEMBER_T(tab_user)
    };
};