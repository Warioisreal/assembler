#ifndef _FILE_FUNC_H_
#define _FILE_FUNC_H_

const size_t MAX_FILENAME_LEN = 128;

static const char* DEFAULT_FILENAME_DATA   = "data.txt";
static const char* DEFAULT_FILENAME_DATA_O = "data_o.txt";


void GetFileName(char* filename, const char* def);

#endif // _FILE_FUNC_H_
