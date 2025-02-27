#include "cell.hpp"
#include <string>


Cell::Cell(Liquid liquid, int quanity):
    liquid(liquid),
    quantity(quanity),
    selected(false)
    {}

auto Cell::ToString() -> std::string{
    // int value = static_cast<int>(Cell::liquid);
    // + std::to_string(value) + ":"

    std::string fg;
    if (this->liquid == Liquid::none) {
        fg = "37";
    }
    else if (this->liquid== Liquid::liquid1) {
        fg = "31";
    }
    else if (this->liquid== Liquid::liquid2) {
        fg = "32";
    }
    else if (this->liquid == Liquid::extract) {
        fg = "33";
    }

    std::string bg;
    if (this->selected) {
        bg = "45";
    } else {
        bg = "49";
    }

    return "\033[" + fg + ";" + bg + "m[" + std::to_string(Cell::quantity) + "]\033[0m";
};
