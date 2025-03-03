#ifndef CELL_HPP
#define CELL_HPP

#include "liquid.cpp"
#include <string>

class Cell
{
    public:
        auto ToString() -> std::string;
        Cell(Liquid liquid = Liquid::none, int quanity = 0);
        bool selected;
        Liquid liquid;
        int quantity;
};

#endif
