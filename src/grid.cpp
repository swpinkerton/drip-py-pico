#include "grid.hpp"
#include <cstdio>
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
            this->map[r][c] = Cell(Liquid::none, 0);
        }
    }
    this->map[0][0].selected = true;

    // Allocate pillar rows
    this->pillars = new Pillar*[numrows + 1]; // Allocate one extra row for column headers

    // Allocate columns
    for (int r = 0; r <= numrows; r++) { // Loop from 0 to numrows inclusive
        if (r == 0) {
            this->pillars[r] = new Pillar[numcols + 1]; // Allocate one extra column for row headers
            for (int c = 0; c < numcols + 1; c++) {
                this->pillars[r][c] = Pillar(c == 0 ? "*" : std::string (1, char(64 +c))); // A = 65, B = 66, etc.
            }
        } else {
            this->pillars[r] = new Pillar[1];
            this->pillars[r][0] = Pillar(std::to_string(r));
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
    for (int r = 0; r <= numrows; r++) { // Loop from 0 to numrows inclusive
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

auto Grid::Row() -> int{
    if (this->xposition == -1){
        printf("row");
        return this->yposition;
    }
    return -1;
}
auto Grid::Column() -> int{
    if (this->yposition == -1){
        printf("column");
        return this->xposition;
    }
    return -1;
}
auto Grid::All() -> bool{
    if (this->xposition == -1 && this->yposition == -1){
        return true;
    }
    return false;
}
void Grid::ChangeRowLiquid(Liquid liquid){
    for (int i =0; i < this->numcols; i++){
        this->map[this->yposition][i].liquid = liquid;
    }
}
void Grid::ChangeColumnLiquid(Liquid liquid){
    for (int i =0; i < this->numrows; i++){
        this->map[i][this->xposition].liquid = liquid;
    }
}
void Grid::ChangeAllLiquid(Liquid liquid){
    for (int i =0; i < this->numcols; i++){
        for (int j =0; j < this->numrows; j++){
            this->map[j][i].liquid = liquid;
        }
    }
}
void Grid::ChangeRowQuantity(double quantity){
    for (int i =0; i < this->numcols; i++){
        this->map[this->yposition][i].quantity = quantity;
    }
}
void Grid::ChangeColumnQuantity(double quantity){
    for (int i =0; i < this->numrows; i++){
        this->map[i][this->xposition].quantity = quantity;
    }
}
void Grid::ChangeAllQuantity(double quantity){
    for (int i = 0; i < this->numcols; i++){
        for (int j =0; j < this->numrows; j++){
            this->map[j][i].quantity = quantity;
        }
    }
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
    for (int c = 0; c<this->numcols + 1; c++){
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
