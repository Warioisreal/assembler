#include <stdio.h>

#include "asm.h"
#include "file_func.h"


int main(void) {
    char data_filename[MAX_FILENAME_LEN]   = "";
    char data_o_filename[MAX_FILENAME_LEN] = "";

    GetFileName(data_filename, DEFAULT_FILENAME_DATA);
    GetFileName(data_o_filename, DEFAULT_FILENAME_DATA_O);

    if (Assembler(data_filename, data_o_filename)) { return 1; }

    return 0;
}
