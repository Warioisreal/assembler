#ifndef _ASSEMBLER_H_
#define _ASSEMBLER_H_

static const size_t CMD_SIZE = 13;

enum class CMD : char {
    CMD_HLT   = 0,
    CMD_PUSH  = 1,
    CMD_POP   = 2,
    CMD_ADD   = 3,
    CMD_SUB   = 4,
    CMD_MUL   = 5,
    CMD_DIV   = 6,
    CMD_POW   = 7,
    CMD_SQRT  = 8,
    CMD_IN    = 9,
    CMD_OUT   = 10,
    CMD_PUSHR = 33,
    CMD_POPR  = 34
};

typedef struct Command {
    const char* text_cmd = "HLT";
    CMD cmd        = CMD::CMD_HLT;
    int int_cmd    = 0;
} cmd;

static const cmd CMD_ARRAY[CMD_SIZE] = {{"HLT",   CMD::CMD_HLT,   0},
                                        {"PUSH",  CMD::CMD_PUSH,  1},
                                        {"POP",   CMD::CMD_POP,   2},
                                        {"ADD",   CMD::CMD_ADD,   3},
                                        {"SUB",   CMD::CMD_SUB,   4},
                                        {"MUL",   CMD::CMD_MUL,   5},
                                        {"DIV",   CMD::CMD_DIV,   6},
                                        {"POW",   CMD::CMD_POW,   7},
                                        {"SQRT",  CMD::CMD_SQRT,  8},
                                        {"IN",    CMD::CMD_IN,    9},
                                        {"OUT",   CMD::CMD_OUT,   10},
                                        {"PUSHR", CMD::CMD_PUSHR, 33},
                                        {"POPR",  CMD::CMD_POPR,  34}
                                        };


int Assembler(const char* filename_in, const char* filename_out, size_t* count);

#endif // _ASSEMBLER_H_
