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
        auto Row() -> int;
        auto Column() -> int;
        auto All() -> bool;
        void ChangeRowLiquid(Liquid liquid);
        void ChangeColumnLiquid(Liquid liquid);
        void ChangeAllLiquid(Liquid liquid);
        void ChangeRowQuantity(double quantity);
        void ChangeColumnQuantity(double quantity);
        void ChangeAllQuantity(double quantity);
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
