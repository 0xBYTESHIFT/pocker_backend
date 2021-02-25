#pragma once
#include <functional>
#include <unordered_map>
#include <memory>

class user;
using user_ptr = std::shared_ptr<user>;

class server {
public:
    using user_pred = std::function<bool(const user_ptr&)>;
    using user_cont = std::unordered_map<size_t, user_ptr>; //id, ptr

    void add_user(const user_ptr& user);
    auto get_user(size_t id) const -> const user_ptr;
    auto get_user(size_t id) -> user_ptr;
    auto get_user(const user_pred& pred) const -> const user_ptr;
    auto get_user(const user_pred& pred) -> user_ptr;
    auto users() const -> const user_cont&;
    auto users() -> user_cont&;
private:
    user_cont users_;
};