#include "server.h"
#include "log.hpp"
#include "tracy_include.h"
#include "user.h"
#include "game.h"

void server::add_user(const user_ptr& user) {
    ZoneScoped;
    auto lgr = get_logger();
    auto prefix = "server::add_user";
    lgr.debug("{} user added, id:{}", prefix, user->id());

    this->users_[user->id()] = user;
}
auto server::get_user(size_t id) const -> const user_ptr {
    ZoneScoped;
    auto lgr = get_logger();
    auto prefix = "server::get_user";

    auto it = this->users_.find(id);
    if (it == users_.end()) {
        lgr.debug("{} user not found, id:{}", prefix, id);
        return nullptr;
    }
    lgr.debug("{} user found, id:{}", prefix, id);
    return it->second;
}
auto server::get_user(size_t id) -> user_ptr {
    ZoneScoped;
    const auto c_this = this;
    const auto c_ptr = c_this->get_user(id);
    return std::const_pointer_cast<user>(c_ptr);
}
auto server::get_user(const user_pred& pred) const -> const user_ptr {
    ZoneScoped;
    auto lgr = get_logger();
    auto prefix = "server::get_user";

    for (const auto& [key, user] : users_) {
        if (pred(user)) {
            lgr.debug("{} user found by predicate, id:{}", prefix, user->id());
            return user;
        }
    }
    lgr.debug("{} user not found by predicate", prefix);
    return nullptr;
}
auto server::get_user(const user_pred& pred) -> user_ptr {
    ZoneScoped;
    const auto c_this = this;
    const auto c_ptr = c_this->get_user(pred);
    return std::const_pointer_cast<user>(c_ptr);
}
auto server::users() const -> const user_cont& {
    ZoneScoped;
    return this->users_;
}
auto server::users() -> user_cont& {
    ZoneScoped;
    return this->users_;
}

auto server::add_game() -> game_ptr{
    auto game = std::make_shared<class game>();
    this->games_.emplace_back(game);
    return game;
}
auto server::games() const -> const game_cont&{
    return this->games_;
}
auto server::games() -> game_cont&{
    return this->games_;
}
