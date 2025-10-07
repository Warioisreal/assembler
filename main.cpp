#include <stdio.h>

#include "asm.h"


int main(void) {
    char data_filename[MAX_FILENAME_LEN]   = "";
    char data_o_filename[MAX_FILENAME_LEN] = "";

    GetFileName(data_filename, DEFAULT_FILENAME_DATA);
    GetFileName(data_o_filename, DEFAULT_FILENAME_DATA_O);

    size_t cmd_count = 0;

    if (Assembler(data_filename, data_o_filename, &cmd_count)) { return 1; }

    return 0;
}
