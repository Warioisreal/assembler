#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "color_lib.h"

#include "file_func.h"


void GetFileName(char* filename, const char* def) {

    assert (filename != nullptr);
    assert (def      != nullptr);

    PRINT_COLOR_VAR(YELLOW, "\nEnter filename or press [enter] to use %s\n", def);

    if (fgets(filename, MAX_FILENAME_LEN, stdin) != nullptr) {

        size_t len = strnlen(filename, MAX_FILENAME_LEN);

        if (len == 1) {
            strncpy(filename, def, MAX_FILENAME_LEN);
        } else if (len > 0 && (filename[len - 1] == '\n')) {
            filename[len - 1] = '\0';
        }
    } else {
        PRINT_COLOR(RED, "\nfgets ERROR\n");
        PRINT_COLOR(YELLOW, "\nused default filename\n");
        strncpy(filename, def, MAX_FILENAME_LEN);
    }
    PRINT_COLOR_VAR(GREEN, "filename: %s\n", filename);
}
