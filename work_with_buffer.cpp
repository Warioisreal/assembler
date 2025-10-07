#include <string.h>
#include <assert.h>


#include "work_with_buffer.h"


void FreeBuffer(char** buffer) {

    assert (buffer != nullptr);

    free(*buffer);
    *buffer = nullptr;
}
