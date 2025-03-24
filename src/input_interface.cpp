#include "cell.hpp"
#include <cstdio>
#include <vector>


enum InputState {
    neutral,
    move,
    liquid
};


struct LiquidCommand {
    Liquid liquid;
    float quantity;
};


struct MoveCommand {
    int row;
    int column;
};


union Command {
    LiquidCommand liquid;
    MoveCommand move;
};


void ClearQueue(std::vector<Command> *commands){
    commands->clear();
}


void SendData(std::vector<Command> *commands){
    // TODO: send the queued commands
    ClearQueue(commands);
}


void NeutralHandler(char c, InputState *state){
    switch (c) {
        case 'm':
            * state = InputState.move;
            break;
        case 'l':
            * state = InputState.liquid;
            break;
        default:
            break;
    }
}


int RowHandler(char c){
    if ('0' <= c && c <= '9'){
        return c - '0';
    }
    return -1;
}


int ColumnHandler(char c){
    if ('a' <= c && c <= 'z'){
        return c - 'a';
    }
    if ('A' <= c && c <= 'Z'){
        return c - 'A';
    }
    return -1;
}


auto MoveHandler (char c, InputState *state) -> MoveCommand{
    int row = RowHandler(c);
    int column = ColumnHandler(c);
    while (row == -1 || column == -1){
        c = getchar();
        int temp_row = RowHandler(c);
        if (temp_row != -1){
            row = temp_row;
        }
        int temp_column = ColumnHandler(c);
        if (temp_column != -1){
            column = temp_column;
        }
    }
    * state = InputState.Nuetral;
    MoveCommand mvc = MoveCommand();
    mvc.row = row;
    mvc.column = column;
    return mvc;
}


auto LiquidHandler (char c, InputState *state) -> LiquidCommand{
    int row = RowHandler(c);
    int column = ColumnHandler(c);
    while (row == -1 || column == -1){
        c = getchar();
        int temp_row = RowHandler(c);
        if (temp_row != -1){
            row = temp_row;
        }
        int temp_column = ColumnHandler(c);
        if (temp_column != -1){
            column = temp_column;
        }
    }
    * state = InputState.Nuetral;
    LiquidCommand lqc = LiquidCommand();

}


void InputInterface(void* p) {
    InputState * state = (InputState *) p;
    std::vector<Command> queue;
    while (1){
        // recieve char
        int c = getchar(); // Non-blocking read
        // check for enter
        if (c == '\r' || c == '\n'){
            SendData(&queue);
        }
        // check for escape button
        if (c == ' '){
            ClearQueue(&queue);
        }
        if (state == InputState.nuetral){
            NeutralHandler(c, state);
        }
        if (state == InputState.move){
            MoveCommand mq = MoveHandler(c, state);
            queue.insert(mq);
        }
        if (state == InputState.liquid){
            LiquidCommand lq = LiquidHandler(c, state);
            queue.insert(lq);
        }
    }
}
