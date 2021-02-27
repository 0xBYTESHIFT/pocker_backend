#pragma once
#include "property.hpp"
#include <memory>
#include <vector>

class player;
using player_ptr = std::shared_ptr<player>;

class game {
public:
    property<std::vector<player_ptr>> players;
};