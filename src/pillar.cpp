#include "pillar.hpp"
#include <string>

Pillar::Pillar(std::string identifer):
    identifer(identifer = " "),
    selected(false)
    {}

auto Pillar::ToString() -> std::string{
    std::string bg;
    if (this->selected) {
        bg = "45";
    } else {
        bg = "49";
    }

    return "\033[37;" + bg + "m[" + this->identifer + "]\033[0m";
};
