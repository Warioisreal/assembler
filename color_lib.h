#ifndef _COLOR_LIB_H_
#define _COLOR_LIB_H_

#include <stdio.h>

#define BASE    "\033[0m"
#define BLACK   "\033[30m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define BLUE    "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN    "\033[36m"
#define GREY    "\033[37m"

#define PRINT_COLOR(color, arg) printf(color arg BASE)
#define PRINT_COLOR_VAR(color, arg, ...) printf(color arg BASE, __VA_ARGS__)

#endif // _COLOR_LIB_H_
