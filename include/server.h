#pragma once
#include "token.hpp"
#include <functional>
#include <memory>
#include <unordered_map>
#include <vector>

class user;
using user_ptr = std::shared_ptr<user>;
class game;
using game_ptr = std::shared_ptr<game>;

class server {
public:
    using user_pred = std::function<bool(const user_ptr&)>;
    using user_cont = std::unordered_map<size_t, user_ptr>; //id, ptr

    using game_cont = std::vector<game_ptr>;

    void add_user(const user_ptr& user);
    auto get_user(size_t id) const -> const user_ptr;
    auto get_user(size_t id) -> user_ptr;
    auto get_user(const user_pred& pred) const -> const user_ptr;
    auto get_user(const user_pred& pred) -> user_ptr;
    auto users() const -> const user_cont&;
    auto users() -> user_cont&;

    auto add_game() -> game_ptr;
    auto games() const -> const game_cont&;
    auto games() -> game_cont&;

private:
    user_cont users_;
    game_cont games_;
};