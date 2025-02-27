#include "grid.hpp"
#include <string>
#include <sys/types.h>


Grid::Grid(int numrows, int numcols) {
    this->numrows = numrows;
    this->numcols = numcols;
    this->xposition = 0;
    this->yposition = 0;

    // Allocate rows
    this->map = new Cell*[numrows];

    // Allocate columns
    for (int r = 0; r < numrows; r++) {
        this->map[r] = new Cell[numcols];  // Allocate each row as an array of Cells
        for (int c = 0; c < numcols; c++) {
            this->map[r][c] = Cell(Liquid::liquid1, 0.0);
        }
    }
    this->map[0][0].selected = true;

    // Allocate pillar rows
    this->pillars = new Pillar*[numrows];

    // Allocate columns
    for (int r = 0; r < numrows; r++) {
        if (r == 0){
            this->pillars[r] = new Pillar[numcols];  // Allocate each row as an array of Cells
            for (int c = 0; c < numcols; c++) {
                this->pillars[r][c] = Pillar(" " + std::to_string(c ? ('A' + c + 1) : '*') + " ");
            }
        }
        else {
            this->pillars[r] = new Pillar[1];
            this->pillars[r][0] = Pillar(" " + std::to_string(r) + " ");
        }

    }
}

Grid::~Grid() {
    // Free each row first
    for (int r = 0; r < numrows; r++) {
        delete[] this->map[r];
    }
    // Free the row array
    delete[] this->map;

    // Free each row first
    for (int r = 0; r < numrows; r++) {
        delete[] this->pillars[r];
    }

    // Free the row array
    delete[] this->pillars;
}

void Grid::ChangeElement(Cell element, int row, int col){
    this->map[row][col] = element;
}

void Grid::ChangeLiquid(Liquid liquid){
    this->map[this->yposition][this->xposition].liquid = liquid;
}

void Grid::ChangeQuantity(double quantity){
    this->map[this->yposition][this->xposition].quantity = quantity;
}

void Grid::MoveCursor(int x, int y){
    if (this->xposition == -1 || this->yposition == -1){
        this->pillars[this->yposition+1][this->xposition+1].selected = false;
    }
    else {
        this->map[this->yposition][this->xposition].selected = false;
    }

    this->xposition += x;
    this->yposition += y;

    if (this->xposition < -1){
        this->xposition = this->numcols-1;
    }
    if (this->yposition < -1){
        this->yposition = this->numrows-1;
    }
    if (this->xposition >= this->numcols){
        this->xposition = -1;
    }
    if (this->yposition >= this->numrows){
        this->yposition = -1;
    }

    if (this->xposition == -1 || this->yposition == -1){
        this->pillars[this->yposition+1][this->xposition+1].selected = true;
    }
    else {
        this->map[this->yposition][this->xposition].selected = true;
    }
}

auto Grid::ToString() -> std::string{
    std::string output;
    for (int c = 0; c<this->numcols; c++){
        output.append(this->pillars[0][c].ToString());
    }
    output += "\n";
    for (int r = 0; r<this->numrows; r++){
        output.append(this->pillars[r+1][0].ToString());
        for (int c = 0; c<this->numcols; c++){
            output.append(this->map[r][c].ToString());
        }
        output += "\n";
    }
    return output;
}
