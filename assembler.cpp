#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/stat.h>
#include <string.h>
#include <ctype.h>

#include "color_lib.h"

#include "assembler.h"
#include "work_with_buffer.h"

static int ReadDataToBuffer(char** buf, const char* filename);
static int WriteFromBufferToDataO(char* buf, const char* filename, size_t* count);


int Assembler(const char* filename_in, const char* filename_out, size_t* count) {

    assert (filename_in  != nullptr);
    assert (filename_out != nullptr);
    assert (count        != nullptr);

    char* buffer = nullptr;

    int error = ReadDataToBuffer(&buffer, filename_in);
    if (error) { return 1; }

    error = WriteFromBufferToDataO(buffer, filename_out, count);
    if (error) { return 1; }

    FreeBuffer(&buffer);
    return 0;
}



static int ReadDataToBuffer(char** buf, const char* filename) {

    assert (buf      != nullptr);
    assert (filename != nullptr);

    struct stat file_stat;
    if (stat(filename, &file_stat) != 0) {
        PRINT_COLOR(RED, "filesize read error\n");
        return 1;
    }
    const size_t filesize = (size_t)(file_stat.st_size);

    FILE* file = fopen(filename, "rb");
    if (file == nullptr) {
        PRINT_COLOR_VAR(RED, "file open error: \"%s\"\n", filename);
        return 1;
    }

    char* buf_ = (char*)calloc(filesize + 1, sizeof(char));
    if (buf_ == nullptr) {
        PRINT_COLOR(RED, "buffer calloc error\n");
        return 1;
    }
    *buf = buf_;

    size_t fread_filesize = fread(*buf, sizeof(char), filesize, file);
    if (filesize > fread_filesize) {
        if (feof(file)) {                       // if reached EOF before filesize
            PRINT_COLOR(RED, "reached EOF\n");
        } else if (ferror(file)) {              // if had readfile error
            PRINT_COLOR(RED, "file read error\n");
        } else {
            PRINT_COLOR(RED, "filesize error: fread.size != filesize\n"); // if fread.size mistake
        }
        FreeBuffer(buf);
        return 1;
    }
    for (size_t pos = 0; pos < filesize; pos++) {
        if ((*buf)[pos] == '\n' || (*buf)[pos] == ' ') {
            (*buf)[pos] = '\0';
        }
    }

    fclose(file);

    return 0;
}


static int WriteFromBufferToDataO(char* buf, const char* filename, size_t* count) {

    assert (filename != nullptr);
    assert (count    != nullptr);

    FILE* file = fopen(filename, "wb");
    if (file == nullptr) {
        PRINT_COLOR_VAR(RED, "file open error: \"%s\"\n", filename);
        return 1;
    }

    char* text_command = buf;
    int run = 1;

    while (run) {
        for (size_t pos = 0; pos < CMD_SIZE; pos++) {
            if (strcmp(text_command, CMD_ARRAY[pos].text_cmd) == 0) {
                if (CMD_ARRAY[pos].cmd == CMD::CMD_HLT) {
                    fprintf(file, "%d", CMD_ARRAY[pos].int_cmd);
                    (*count)++;
                    run = 0; // exit while
                } else if (CMD_ARRAY[pos].cmd == CMD::CMD_PUSH || CMD_ARRAY[pos].cmd == CMD::CMD_PUSHR || CMD_ARRAY[pos].cmd == CMD::CMD_POPR) {
                    fprintf(file, "%d ", CMD_ARRAY[pos].int_cmd);
                    (*count)++;
                    text_command = strchr(text_command, '\0') + 1;

                    if (isdigit(*text_command)) { // вопросик про проверку числа
                        fprintf(file, "%s ", text_command);
                        (*count)++;
                    } else {
                        fclose(file);
                        return 1; // error
                    }
                } else {
                    fprintf(file, "%d ", CMD_ARRAY[pos].int_cmd);
                    (*count)++;
                }
                break;
            }
            if (pos == CMD_SIZE - 1) {
                fclose(file);
                return 1; // error
            }
        }
        text_command = strchr(text_command, '\0') + 1;
    }

    fclose(file);

    return 0;
}
