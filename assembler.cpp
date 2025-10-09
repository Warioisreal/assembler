#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/stat.h>
#include <string.h>
#include <ctype.h>

#include "color_lib.h"

#include "assembler.h"
#include "work_with_buffer.h"

static int ReadDataToBuffer(char** buf, size_t* file_size, const char* filename);
static int ParseBufferToBytecode(char* buffer, size_t buffer_size, int** bytecode, size_t* code_size);
static int WriteBytecodeToFile(int* bytecode, size_t code_size, const char* filename);


int Assembler(const char* filename_in, const char* filename_out) {
    assert(filename_in  != nullptr);
    assert(filename_out != nullptr);

    char* buffer = nullptr;
    size_t file_size = 0;

    int error = ReadDataToBuffer(&buffer, &file_size, filename_in);
    if (error) { return 1; }

    int* bytecode = nullptr;
    size_t code_size = 0;

    error = ParseBufferToBytecode(buffer, file_size, &bytecode, &code_size);
    FreeBuffer(&buffer);

    if (error) { return 1; }

    error = WriteBytecodeToFile(bytecode, code_size, filename_out);
    free(bytecode);

    if (error) { return 1; }

    return 0;
}

//----------------------------------------------------------------------------------

static int ReadDataToBuffer(char** buf, size_t* file_size, const char* filename) {
    assert(buf       != nullptr);
    assert(file_size != nullptr);
    assert(filename  != nullptr);

    struct stat file_stat;
    if (stat(filename, &file_stat) != 0) {
        PRINT_COLOR(RED, "filesize read error\n");
        return 1;
    }
    *file_size = (size_t)(file_stat.st_size);

    FILE* file = fopen(filename, "rb");
    if (file == nullptr) {
        PRINT_COLOR_VAR(RED, "file open error: \"%s\"\n", filename);
        return 1;
    }

    char* buf_ = (char*)calloc(*file_size + 1, sizeof(char));
    if (buf_ == nullptr) {
        PRINT_COLOR(RED, "buffer calloc error\n");
        return 1;
    }
    *buf = buf_;

    size_t fread_filesize = fread(*buf, sizeof(char), *file_size, file);
    if (*file_size > fread_filesize) {
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
    for (size_t pos = 0; pos < *file_size; pos++) {
        if ((*buf)[pos] == '\n' || (*buf)[pos] == ' ') {
            (*buf)[pos] = '\0';
        }
    }

    fclose(file);

    return 0;
}

//----------------------------------------------------------------------------------

static int ParseBufferToBytecode(char* buffer, size_t buffer_size, int** bytecode, size_t* code_size) {
    assert(buffer != nullptr);
    assert(bytecode != nullptr);
    assert(code_size != nullptr);

    int* temp_bytecode = (int*)calloc(buffer_size / 2, sizeof(int));
    if (temp_bytecode == nullptr) {
        PRINT_COLOR(RED, "bytecode calloc error\n");
        return 1;
    }

    size_t count = 0;
    char* current_pos = buffer;
    char* buffer_end = buffer + buffer_size;
    int command_found = 0; // check result cmd in CMD_ARRAY[]

    while (current_pos < buffer_end && *current_pos != '\0') {

        while (current_pos < buffer_end && isspace(*current_pos)) { current_pos++; }

        if (current_pos >= buffer_end) break;

        command_found = 0;

        for (size_t i = 0; i < CMD_SIZE; i++) {
            // check text command
            if (strncmp(current_pos, CMD_ARRAY[i].text_cmd, CMD_ARRAY[i].cmd_len) == 0 &&                               // check cmd chars
                (isspace(*(current_pos + CMD_ARRAY[i].cmd_len)) || *(current_pos + CMD_ARRAY[i].cmd_len) == '\0')) {    // check after cmd chars

                temp_bytecode[count++] = CMD_ARRAY[i].int_cmd; // write cmd to temp_bytecode
                command_found = 1;

                current_pos += CMD_ARRAY[i].cmd_len;

                if (CMD_ARRAY[i].cmd == CMD::CMD_PUSH ||
                    CMD_ARRAY[i].cmd == CMD::CMD_JMP ||
                    CMD_ARRAY[i].cmd == CMD::CMD_JB ||
                    CMD_ARRAY[i].cmd == CMD::CMD_JBE ||
                    CMD_ARRAY[i].cmd == CMD::CMD_JA ||
                    CMD_ARRAY[i].cmd == CMD::CMD_JAE ||
                    CMD_ARRAY[i].cmd == CMD::CMD_JE ||
                    CMD_ARRAY[i].cmd == CMD::CMD_JNE ||
                    CMD_ARRAY[i].cmd == CMD::CMD_PUSHR ||
                    CMD_ARRAY[i].cmd == CMD::CMD_POPR) {

                    while (current_pos < buffer_end && isspace(*current_pos)) { current_pos++; }

                    // check cmd arg
                    if (current_pos >= buffer_end || !isdigit(*current_pos)) { // bad number check
                        printf("%d | %d\n", *current_pos, isdigit(*current_pos));
                        PRINT_COLOR(RED, "Expected number after command\n");
                        free(temp_bytecode);
                        return 1;
                    }
                    // parse number
                    char* end_ptr = nullptr;
                    int value = strtol(current_pos, &end_ptr, 10);

                    if (end_ptr == current_pos) {
                        PRINT_COLOR(RED, "Failed to parse number\n");
                        free(temp_bytecode);
                        return 1;
                    }

                    temp_bytecode[count++] = value;  // write arg to temp_bytecode
                    current_pos = end_ptr;
                }

                break;
            }
        }

        if (!command_found) {
            PRINT_COLOR_VAR(RED, "Unknown command: %s\n", current_pos);
            free(temp_bytecode);
            return 1;
        }
    }

    if (count > 0) {
        int* buf = (int*)realloc(temp_bytecode, count * sizeof(int)); // bytecode allocation with data write
        if (buf == nullptr) {
            free(temp_bytecode);
            PRINT_COLOR(RED, "bytecode allocation error\n");
            return 1;
        } else {
            *bytecode = buf;
            *code_size = count;
        }

    } else {
        free(temp_bytecode);
        PRINT_COLOR(RED, "No commands found in file\n");
        return 1;
    }

    return 0;
}

//----------------------------------------------------------------------------------

static int WriteBytecodeToFile(int* bytecode, size_t code_size, const char* filename) {
    assert(filename != nullptr);
    assert(bytecode != nullptr);

    FILE* file = fopen(filename, "wb");
    if (file == nullptr) {
        PRINT_COLOR_VAR(RED, "file open error: \"%s\"\n", filename);
        return 1;
    }

    FileHeader header = {SIGNATURE, VERSION, code_size}; // make file header

    // write file header
    if (fwrite(&header, sizeof(FileHeader), 1, file) != 1) {
    PRINT_COLOR(RED, "Failed to write file header\n");
    fclose(file);
    return 1;
    }
    // write bytecode
    if (fwrite(bytecode, sizeof(int), code_size, file) != code_size) {
        PRINT_COLOR(RED, "Failed to write bytecode\n");
        fclose(file);
        return 1;
    }

    fclose(file);

    PRINT_COLOR_VAR(GREEN, "Successfully assembled: %zu commands written to %s\n", code_size, filename);
    return 0;

    return 0;
}
