#ifndef COMMANDS_HPP
#define COMMANDS_HPP

#include "liquid.cpp"

struct LiquidCommand {
    Liquid liquid;
    int quantity;
};

struct MoveCommand {
    int row;
    int column;
};

enum CommandType {
    COMMAND_MOVE,
    COMMAND_LIQUID
};

struct Command {
    CommandType type;
    union {
        LiquidCommand liquid;
        MoveCommand move;
    } data;
};

#endif
