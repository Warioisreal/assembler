#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/stat.h>
#include <string.h>
#include <ctype.h>

#include "color_lib.h"

#include "assembler.h"
#include "work_with_buffer.h"

static asm_error_t ReadDataToBuffer(char** buf, const char* filename, size_t* filesize);
static asm_error_t FillLabelsArray(char* buf, size_t* count, int labels[10], size_t filesize);
static asm_error_t WriteFromBufferToDataO(char* buf, const char* filename, size_t count, const int labels[10], FILE* asm_lst, size_t filesize);
static void PrintLabelsArray(const int labels[10], FILE* asm_lst);
static void PrintListingHeader(FILE* asm_lst, size_t count);
static void PrintCommandListing(FILE* asm_lst, size_t index, const char* mnemonic, const char* operand, stack_elem_t code);
static void WriteCMD(size_t* buf_index, char** text_com, size_t* bytes_processed, size_t* listing_index,
                     bool* command_found, stack_elem_t** code_buffer, FILE* asm_lst,
                     const int labels[10], char* buf, size_t filesize);

void DumpAsmError(asm_error_t error) {
    switch (error) {
        case asm_error_t::ASM_OK:
            PRINT_COLOR(GREEN, "Assembler: OK\n");
            break;
        case asm_error_t::ASM_FILE_OPEN_ERROR:
            PRINT_COLOR(RED, "Assembler: File open error\n");
            break;
        case asm_error_t::ASM_FILE_READ_ERROR:
            PRINT_COLOR(RED, "Assembler: File read error\n");
            break;
        case asm_error_t::ASM_BUFFER_ALLOC_ERROR:
            PRINT_COLOR(RED, "Assembler: Buffer allocation error\n");
            break;
        case asm_error_t::ASM_UNKNOWN_COMMAND:
            PRINT_COLOR(RED, "Assembler: Unknown command\n");
            break;
        case asm_error_t::ASM_INVALID_ARGUMENT:
            PRINT_COLOR(RED, "Assembler: Invalid argument\n");
            break;
        case asm_error_t::ASM_LABEL_ERROR:
            PRINT_COLOR(RED, "Assembler: Label error\n");
            break;
        case asm_error_t::ASM_FILE_WRITE_ERROR:
            PRINT_COLOR(RED, "Assembler: File write error\n");
            break;
        default:
            PRINT_COLOR(RED, "Assembler: Unknown error\n");
            break;
    }
}

//----------------------------------------------------------------------------------

int Assembler(const char* filename_in, const char* filename_out, size_t* count) {

    assert (filename_in  != nullptr);
    assert (filename_out != nullptr);
    assert (count        != nullptr);

    char* buffer = nullptr;
    size_t filesize = 0;

    asm_error_t error = ReadDataToBuffer(&buffer, filename_in, &filesize);
    if (error != asm_error_t::ASM_OK) {
        DumpAsmError(error);
        return 1;
    }

    FILE* asm_lst = fopen(ASSEMBLER_LISTING_FILENAME, "wb");
    if (asm_lst == nullptr) {
        PRINT_COLOR_VAR(RED, "file open error: \"%s\"\n", ASSEMBLER_LISTING_FILENAME);
        DumpAsmError(asm_error_t::ASM_FILE_OPEN_ERROR);
        return 1;
    }

    int labels[10] = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1};

    error = FillLabelsArray(buffer, count, labels, filesize);
    if (error != asm_error_t::ASM_OK) {
        DumpAsmError(error);
        fclose(asm_lst);
        return 1;
    }

    error = WriteFromBufferToDataO(buffer, filename_out, *count, labels, asm_lst, filesize);
    if (error != asm_error_t::ASM_OK) {
        DumpAsmError(error);
        fclose(asm_lst);
        return 1;
    }

    fclose(asm_lst);
    FreeBuffer(&buffer);

    DumpAsmError(asm_error_t::ASM_OK);
    return 0;
}

//----------------------------------------------------------------------------------

static asm_error_t ReadDataToBuffer(char** buf, const char* filename, size_t* filesize) {
    assert (buf      != nullptr);
    assert (filename != nullptr);

    struct stat file_stat;
    if (stat(filename, &file_stat) != 0) {
        PRINT_COLOR_VAR(RED, "filesize read error: \"%s\"\n", filename);
        return asm_error_t::ASM_FILE_READ_ERROR;
    }
    *filesize = (size_t)(file_stat.st_size);

    FILE* file = fopen(filename, "rb");
    if (file == nullptr) {
        PRINT_COLOR_VAR(RED, "file open error: \"%s\"\n", filename);
        return asm_error_t::ASM_FILE_OPEN_ERROR;
    }

    char* buf_ = (char*)calloc(*filesize + 1, sizeof(char));
    if (buf_ == nullptr) {
        PRINT_COLOR(RED, "buffer calloc error\n");
        fclose(file);
        return asm_error_t::ASM_BUFFER_ALLOC_ERROR;
    }
    *buf = buf_;

    size_t fread_filesize = fread(*buf, sizeof(char), *filesize, file);
    if (*filesize > fread_filesize) {
        if (feof(file)) {
            PRINT_COLOR_VAR(RED, "reached EOF: \"%s\"\n", filename);
        } else if (ferror(file)) {
            PRINT_COLOR_VAR(RED, "file read error: \"%s\"\n", filename);
        } else {
            PRINT_COLOR_VAR(RED, "(fread.size != filesize) filesize error:  \"%s\"\n", filename);
        }
        FreeBuffer(buf);
        fclose(file);
        return asm_error_t::ASM_FILE_READ_ERROR;
    }
    bool is_comment = false;
    for (size_t pos = 0; pos < *filesize; pos++) {
        if ((*buf)[pos] == ';') {
            is_comment = true;
        } else
        if (is_comment == false && (*buf)[pos] == ' ') {
            (*buf)[pos] = '\0';
        } else
        if ((*buf)[pos] == '\n') {
            (*buf)[pos] = '\0';
            is_comment = false;
        }
    }

    fclose(file);

    return asm_error_t::ASM_OK;
}

//----------------------------------------------------------------------------------

static asm_error_t FillLabelsArray(char* buf, size_t* count, int labels[10], size_t filesize) {
    assert (buf    != nullptr);
    assert (count  != nullptr);
    assert (labels != nullptr);

    stack_elem_t atof_result = 0;
    bool command_found = false;
    size_t bytes_processed = 0;
    char* text_command = buf;

    bytes_processed = (size_t)(text_command - buf);
    while (text_command[0] == '\0' && bytes_processed < filesize) { text_command++; bytes_processed++; } // skip spaces in txt_cmd

    while (bytes_processed < filesize) {
        command_found = false;

        for (size_t pos = 0; pos < CMD_SIZE; pos++) {
            if (strcmp(text_command, CMD_ARRAY[pos].text_cmd) == 0) {
                command_found = true;

                if (CMD_ARRAY[pos].cmd == CMD::CMD_PUSH  || CMD_ARRAY[pos].cmd == CMD::CMD_JMP  || \
                    CMD_ARRAY[pos].cmd == CMD::CMD_JB    || CMD_ARRAY[pos].cmd == CMD::CMD_JBE  || \
                    CMD_ARRAY[pos].cmd == CMD::CMD_JA    || CMD_ARRAY[pos].cmd == CMD::CMD_JAE  || \
                    CMD_ARRAY[pos].cmd == CMD::CMD_JE    || CMD_ARRAY[pos].cmd == CMD::CMD_JNE  || \
                    CMD_ARRAY[pos].cmd == CMD::CMD_PUSHR || CMD_ARRAY[pos].cmd == CMD::CMD_POPR || \
                    CMD_ARRAY[pos].cmd == CMD::CMD_PUSHM || CMD_ARRAY[pos].cmd == CMD::CMD_POPM || \
                    CMD_ARRAY[pos].cmd == CMD::CMD_CALL)
                {
                    (*count)++;
                    text_command = strchr(text_command, '\0') + 1;
                    bytes_processed = (size_t)(text_command - buf);
                    while (text_command[0] == '\0' && bytes_processed < filesize) { text_command++; bytes_processed++; } // skip spaces in txt_cmd

                    atof_result = atof(text_command);
                    if (text_command[0] == ':' || (text_command[0] == '0' && text_command[1] == '\0' && atof_result == 0) || atof_result) {
                        (*count)++;
                    } else {
                        PRINT_COLOR_VAR(RED, "Invalid argument: %s\n", text_command);
                        return asm_error_t::ASM_INVALID_ARGUMENT;
                    }
                } else { (*count)++; }
                break;
            }
        }
        if (command_found == false) {
            if (text_command[0] == ':') {
                labels[text_command[1] - '0'] = (int)(++(*count));
            } else if (text_command[0] == ';') {
                text_command = strchr(text_command, '\0') + 1;
                bytes_processed = (size_t)(text_command - buf);
            } else {
                PRINT_COLOR_VAR(RED, "Unknown command: %s\n", text_command);
                return asm_error_t::ASM_UNKNOWN_COMMAND;
            }
        }
        text_command = strchr(text_command, '\0') + 1;
        bytes_processed = (size_t)(text_command - buf);
        while (text_command[0] == '\0' && bytes_processed < filesize) { text_command++; bytes_processed++; } // skip spaces in txt_cmd
    }

    return asm_error_t::ASM_OK;
}

//----------------------------------------------------------------------------------

static asm_error_t WriteFromBufferToDataO(char* buf, const char* filename, const size_t count, const int labels[10], FILE* asm_lst, size_t filesize) {
    assert(buf      != nullptr);
    assert(filename != nullptr);
    assert(labels   != nullptr);
    FILE* file = fopen(filename, "wb");
    if (file == nullptr) {
        PRINT_COLOR_VAR(RED, "file open error: \"%s\"\n", filename);
        return asm_error_t::ASM_FILE_OPEN_ERROR;
    }

    stack_elem_t* code_buffer = (stack_elem_t*)calloc(count, sizeof(stack_elem_t));
    if (code_buffer == nullptr) {
        PRINT_COLOR(RED, "code_buffer calloc error\n");
        fclose(file);
        return asm_error_t::ASM_BUFFER_ALLOC_ERROR;
    }

    size_t buf_index = 0;

    FileHeader header = {SIGNATURE, VERSION, count};

    if (fwrite(&header, sizeof(FileHeader), 1, file) != 1) {
        PRINT_COLOR(RED, "Error writing file header\n");
        free(code_buffer);
        fclose(file);
        return asm_error_t::ASM_FILE_WRITE_ERROR;
    }

    PrintLabelsArray(labels, asm_lst);
    PrintListingHeader(asm_lst, count);

    char* text_command     = buf;
    size_t bytes_processed = 0;
    size_t listing_index   = 0;
    bool command_found     = false;

    while (bytes_processed < filesize) {
        //printf("%p | %p | %p | %zu | %zu\n", buf, text_command, buf + filesize, bytes_processed, filesize);
        //printf("%c | %s | %d\n", text_command[0], text_command, text_command[0] == ':');

        if (bytes_processed >= filesize || text_command >= buf + filesize) {
            printf("seg er\n");
            break;
        }

        if (text_command[0] == ':') {  // Метка
            int label_index = *(text_command + 1) - '0';
            fprintf(asm_lst, "%-6zu                                  ; LABEL :%d (address: %d)\n",
                    listing_index, label_index, labels[label_index]);
            text_command = strchr(text_command, '\0') + 1;
            bytes_processed = (size_t)(text_command - buf);
            while (text_command[0] == '\0' && bytes_processed < filesize) { text_command++; bytes_processed++; } // skip spaces in txt_cmd
            listing_index++;
            code_buffer[buf_index++] = CMD_ARRAY[(int)(CMD::CMD_NOP)].int_cmd;
            continue;
        }

        if (text_command[0] == ';') { // Комментарий
            fprintf(asm_lst, "%-6zu                                  ; COMMENT: %s\n", listing_index, text_command + 1);
            text_command = strchr(text_command, '\0') + 1;
            bytes_processed = (size_t)(text_command - buf);
            while (text_command[0] == '\0' && bytes_processed < filesize) { text_command++; bytes_processed++; } // skip spaces in txt_cmd
            listing_index++;
            code_buffer[buf_index++] = CMD_ARRAY[(int)(CMD::CMD_NOP)].int_cmd;
            continue;
        }

        WriteCMD(&buf_index, &text_command, &bytes_processed, &listing_index, &command_found, &code_buffer, asm_lst, labels, buf, filesize);

        if (!command_found) {
            PRINT_COLOR_VAR(RED, "Unknown command in write: %s\n", text_command);
            free(code_buffer);
            fclose(file);
            return asm_error_t::ASM_UNKNOWN_COMMAND;
        }
    }

    if (fwrite(code_buffer, sizeof(stack_elem_t), count, file) != count) {
        PRINT_COLOR(RED, "Error writing commands\n");
        free(code_buffer);
        fclose(file);
        return asm_error_t::ASM_FILE_WRITE_ERROR;
    }

    free(code_buffer);

    fclose(file);

    return asm_error_t::ASM_OK;
}

//----------------------------------------------------------------------------------

static void PrintLabelsArray(const int labels[10], FILE* asm_lst) {
    fprintf(asm_lst, "=== LABEL TABLE ===\n");
    fprintf(asm_lst, "Idx  Address\n");
    fprintf(asm_lst, "---  -------\n");
    for (size_t pos = 0; pos < 10; pos++) {
        fprintf(asm_lst, ":%-2zu   %-6d\n", pos, labels[pos]);
    }
    fprintf(asm_lst, "\n");
}

//----------------------------------------------------------------------------------

static void PrintListingHeader(FILE* asm_lst, size_t count) {
    fprintf(asm_lst, "ASSEMBLER LISTING\n");
    fprintf(asm_lst, "=================\n");
    fprintf(asm_lst, "Total commands: %zu\n", count);
    fprintf(asm_lst, "Signature: 0x%zX\n", SIGNATURE);
    fprintf(asm_lst, "Version: %zu\n\n", VERSION);

    fprintf(asm_lst, "Line    Code    Mnemonic    Operand     ; Comment\n");
    fprintf(asm_lst, "------  ------  ----------  ----------  ------------\n");
}

//----------------------------------------------------------------------------------

static void PrintCommandListing(FILE* asm_lst, size_t index, const char* mnemonic, const char* operand, stack_elem_t code) {
    fprintf(asm_lst, "%-6zu  %-6.0f  %-10s  %-10s\n", index, code, mnemonic, operand);
}

//----------------------------------------------------------------------------------

static void WriteCMD(size_t* buf_index, char** text_com, size_t* bytes_processed, size_t* listing_index,
                     bool* command_found, stack_elem_t** code_buffer, FILE* asm_lst,
                     const int labels[10], char* buf, size_t filesize) {
    *command_found = false;

    for (size_t pos = 0; pos < CMD_SIZE; pos++) {
        if (strcmp(*text_com, CMD_ARRAY[pos].text_cmd) == 0) {
            *command_found = true;

            if (CMD_ARRAY[pos].cmd == CMD::CMD_PUSH  || CMD_ARRAY[pos].cmd == CMD::CMD_JMP  ||
                CMD_ARRAY[pos].cmd == CMD::CMD_JB    || CMD_ARRAY[pos].cmd == CMD::CMD_JBE  ||
                CMD_ARRAY[pos].cmd == CMD::CMD_JA    || CMD_ARRAY[pos].cmd == CMD::CMD_JAE  ||
                CMD_ARRAY[pos].cmd == CMD::CMD_JE    || CMD_ARRAY[pos].cmd == CMD::CMD_JNE  ||
                CMD_ARRAY[pos].cmd == CMD::CMD_PUSHR || CMD_ARRAY[pos].cmd == CMD::CMD_POPR ||
                CMD_ARRAY[pos].cmd == CMD::CMD_PUSHM || CMD_ARRAY[pos].cmd == CMD::CMD_POPM ||
                CMD_ARRAY[pos].cmd == CMD::CMD_CALL)
            { // Команда с аргументом

                *text_com = strchr(*text_com, '\0') + 1;
                *bytes_processed = (size_t)((*text_com) - buf);
                while ((*text_com)[0] == '\0' && *bytes_processed < filesize) { (*text_com)++; (*bytes_processed)++; } // skip spaces in txt_cmd

                if ((*text_com)[0] == ':') { // Аргумент - метка
                    int label_index = (*text_com)[1] - '0';
                    (*code_buffer)[(*buf_index)++] = CMD_ARRAY[pos].int_cmd;
                    (*code_buffer)[(*buf_index)++] = (stack_elem_t)(labels[label_index]);

                    PrintCommandListing(asm_lst,
                                        *listing_index,
                                        CMD_ARRAY[pos].text_cmd,
                                        *text_com,
                                        CMD_ARRAY[pos].int_cmd);
                } else { // Аргумент - число
                    (*code_buffer)[(*buf_index)++] = CMD_ARRAY[pos].int_cmd;
                    (*code_buffer)[(*buf_index)++] = (stack_elem_t)atof(*text_com);

                    PrintCommandListing(asm_lst,
                                        *listing_index,
                                        CMD_ARRAY[pos].text_cmd,
                                        *text_com,
                                        CMD_ARRAY[pos].int_cmd);
                }
            } else { // Команда без аргументов
                printf("1\n");
                (*code_buffer)[(*buf_index)++] = CMD_ARRAY[pos].int_cmd;
                printf("%s | ", CMD_ARRAY[pos].text_cmd);
                printf("%lg\n", CMD_ARRAY[pos].int_cmd);
                //PrintCommandListing(asm_lst,
                //                    *listing_index,
                //                    CMD_ARRAY[pos].text_cmd,
                //                    "",
                //                    CMD_ARRAY[pos].int_cmd);
            }
            *text_com = strchr(*text_com, '\0') + 1;
            *bytes_processed = (size_t)((*text_com) - buf);
            while ((*text_com)[0] == '\0' && *bytes_processed < filesize) { (*text_com)++; (*bytes_processed)++; } // skip spaces in txt_cmd

            (*listing_index)++;
            break;
        }
    }
}
