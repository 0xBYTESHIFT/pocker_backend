#pragma once
#include "components/property.hpp"
#include <string>

class player {
public:
    property<std::string> name;
    property<std::string> token; //thing for referring to
    size_t bank;
};