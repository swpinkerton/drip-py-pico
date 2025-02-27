#ifndef GRID_HPP
#define GRID_HPP

#include "cell.hpp"
#include "pillar.hpp"
#include <string>


class Grid{
    public:
        Grid(int numrows, int numcols);
        ~Grid();
        auto ToString() -> std::string;
        void ChangeElement(Cell element, int row, int col);
        void ChangeLiquid(Liquid liquid);
        void ChangeQuantity(double quantity);
        void MoveCursor(int x, int y);

    private:
        int numrows;
        int numcols;
        Cell** map;
        int xposition;
        int yposition;
        Pillar** pillars;
};
#endif
