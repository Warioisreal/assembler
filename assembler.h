#ifndef _ASSEMBLER_H_
#define _ASSEMBLER_H_

#include "../stack/stack.h"

static const size_t CMD_SIZE = 25;
static const size_t SIGNATURE = 0x4D5341; // "ASM" в little-endian
static const size_t VERSION = 1;
static const char* ASSEMBLER_LISTING_FILENAME = "assembler_list.txt";

struct FileHeader {
    size_t signature = 0;
    size_t version   = 0;
    size_t data_size = 0;
};

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
    CMD_JMP   = 11,
    CMD_JB    = 12,
    CMD_JBE   = 13,
    CMD_JA    = 14,
    CMD_JAE   = 15,
    CMD_JE    = 16,
    CMD_JNE   = 17,
    CMD_CALL  = 18,
    CMD_RET   = 19,
    CMD_PUSHR = 20,
    CMD_POPR  = 21,
    CMD_PUSHM = 22,
    CMD_POPM  = 23,
    CMD_NOP   = 24
};

typedef enum class ASM_ERROR : char {
    ASM_OK                 = 0,
    ASM_FILE_OPEN_ERROR    = 1,
    ASM_FILE_READ_ERROR    = 2,
    ASM_BUFFER_ALLOC_ERROR = 3,
    ASM_UNKNOWN_COMMAND    = 4,
    ASM_INVALID_ARGUMENT   = 5,
    ASM_LABEL_ERROR        = 6,
    ASM_FILE_WRITE_ERROR   = 7
} asm_error_t;

typedef struct Command {
    const char* text_cmd    = "HLT";
    size_t cmd_len          = 0;
    CMD cmd                 = CMD::CMD_HLT;
    stack_elem_t int_cmd    = 0;
} cmd;

static const cmd CMD_ARRAY[CMD_SIZE] = {
    {"HLT",   3, CMD::CMD_HLT,   0},
    {"PUSH",  4, CMD::CMD_PUSH,  1},
    {"POP",   3, CMD::CMD_POP,   2},
    {"ADD",   3, CMD::CMD_ADD,   3},
    {"SUB",   3, CMD::CMD_SUB,   4},
    {"MUL",   3, CMD::CMD_MUL,   5},
    {"DIV",   3, CMD::CMD_DIV,   6},
    {"POW",   3, CMD::CMD_POW,   7},
    {"SQRT",  4, CMD::CMD_SQRT,  8},
    {"IN",    2, CMD::CMD_IN,    9},
    {"OUT",   3, CMD::CMD_OUT,   10},
    {"JMP",   3, CMD::CMD_JMP,   11},
    {"JB",    2, CMD::CMD_JB,    12},
    {"JBE",   3, CMD::CMD_JBE,   13},
    {"JA",    2, CMD::CMD_JA,    14},
    {"JAE",   3, CMD::CMD_JAE,   15},
    {"JE",    2, CMD::CMD_JE,    16},
    {"JNE",   3, CMD::CMD_JNE,   17},
    {"CALL",  4, CMD::CMD_CALL,  18},
    {"RET",   3, CMD::CMD_RET,   19},
    {"PUSHR", 5, CMD::CMD_PUSHR, 20},
    {"POPR",  4, CMD::CMD_POPR,  21},
    {"PUSHM", 5, CMD::CMD_PUSHM, 22},
    {"POPM",  4, CMD::CMD_POPM,  23},
    {"NOP",   3, CMD::CMD_NOP,   24}
};

int Assembler(const char* filename_in, const char* filename_out, size_t* count);
void DumpAsmError(asm_error_t error);

#endif // _ASSEMBLER_H_
