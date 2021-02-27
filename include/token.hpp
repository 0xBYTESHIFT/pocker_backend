#pragma once
#include <algorithm>
#include <random>
#include <unordered_set>
#include <string>

class token {
public:
    operator const std::string&() const {
        return value_;
    }

    bool operator==(const std::string& rhs) const {
        return this->value_ == rhs;
    }

protected:
    friend class token_gen;
    std::string value_;
};

class token_gen {
public:
    static auto generate() -> token {
        token result;
        static std::random_device rd;
        static std::mt19937 p_gen = std::mt19937(rd());
        static std::uniform_int_distribution<unsigned> dist =
            std::uniform_int_distribution<unsigned>(0, alphabet_.size() - 1);

        result.value_.resize(token_len_, ' ');
        bool end = false;
        do {
            std::generate(result.value_.begin(), result.value_.end(),
                          [&]() { return alphabet_.at(dist(p_gen)); });
            if (tokens_.find(result.value_) == tokens_.end()) {
                end = true;
                tokens_.emplace(result.value_);
            }
        } while (!end);
        return result;
    }

private:
    const static inline std::string alphabet_ =
        "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890";
    const static size_t token_len_ = 8;
    static inline std::unordered_set<std::string> tokens_;
};