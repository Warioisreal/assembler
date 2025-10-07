CFLAGS = -D _DEBUG -ggdb3 -std=c++17 -O0 -Wall -Wextra -Weffc++ -Wc++14-compat -Wmissing-declarations \
         -Wcast-align -Wcast-qual -Wchar-subscripts -Wconversion -Wctor-dtor-privacy -Wempty-body \
         -Wfloat-equal -Wformat-nonliteral -Wformat-security -Wformat-signedness -Wformat=2 -Winline \
         -Wnon-virtual-dtor -Woverloaded-virtual -Wpacked -Wpointer-arith -Winit-self -Wredundant-decls \
         -Wshadow -Wsign-conversion -Wsign-promo -Wstrict-overflow=2 -Wsuggest-override -Wswitch-default \
         -Wswitch-enum -Wundef -Wunreachable-code -Wunused -Wvariadic-macros \
         -Wno-missing-field-initializers -Wno-narrowing -Wno-old-style-cast -Wno-varargs -Wstack-protector \
         -fcheck-new -fsized-deallocation -fstack-protector -fstrict-overflow -fno-omit-frame-pointer \
         -Wlarger-than=8192 -fPIE -Werror=vla \
         #-fsanitize=address,alignment,bool,bounds,enum,float-cast-overflow,float-divide-by-zero,integer-divide-by-zero,nonnull-attribute,null,return,returns-nonnull-attribute,shift,signed-integer-overflow,undefined,unreachable,vla-bound,vptr
DEBUG_FLAGS = -DDEBUG
LDFLAGS =
CC = g++

LIB_SOURCES = file_func.cpp assembler.cpp work_with_buffer.cpp
APP_SOURCES = main.cpp

BUILD_TYPE ?= release

OBJDIR_RELEASE = obj/release
OBJDIR_DEBUG = obj/debug

EXECUTABLE_RELEASE = binary_file
EXECUTABLE_DEBUG = binary_file_debug

ASM_LIB_DIR = lib
ASM_INCLUDE_DIR = include
ASM_LIB_NAME = libassembler.a

MAIN_HEADER = asm.h

ifeq ($(BUILD_TYPE), debug)
    CFLAGS += $(DEBUG_FLAGS)
    OBJDIR = $(OBJDIR_DEBUG)
    EXECUTABLE = $(EXECUTABLE_DEBUG)
else
    OBJDIR = $(OBJDIR_RELEASE)
    EXECUTABLE = $(EXECUTABLE_RELEASE)
endif

LIB_OBJECTS = $(addprefix $(OBJDIR)/, $(LIB_SOURCES:.cpp=.o))
APP_OBJECTS = $(addprefix $(OBJDIR)/, $(APP_SOURCES:.cpp=.o))
OBJECTS = $(ASM_LIB_OBJECTS) $(APP_OBJECTS)

all: release

debug:
	$(MAKE) BUILD_TYPE=debug $(EXECUTABLE_DEBUG)

release:
	$(MAKE) BUILD_TYPE=release $(EXECUTABLE_RELEASE)


lib: $(ASM_LIB_DIR)/$(ASM_LIB_NAME)

debug_lib:
	$(MAKE) BUILD_TYPE=debug lib

release_lib:
	$(MAKE) BUILD_TYPE=release lib

$(EXECUTABLE): $(APP_OBJECTS) $(ASM_LIB_DIR)/$(ASM_LIB_NAME)
	$(CC) $(LDFLAGS) $(APP_OBJECTS) -L$(ASM_LIB_DIR) -lassembler -o $@


$(ASM_LIB_DIR)/$(ASM_LIB_NAME): $(LIB_OBJECTS)
	mkdir -p $(ASM_LIB_DIR)
	mkdir -p $(ASM_INCLUDE_DIR)
	ar rcs $@ $^

    # Подключается [project].h файл со всеми include в main.cpp
	cp $(MAIN_HEADER) $(ASM_INCLUDE_DIR)/

    # Если нужны .h файлы проекта
    #cp *.h $(ASM_INCLUDE_DIR)/ 2>/dev/null || true

# @ нужна для того, чтобы команда не выводилась в терминал
$(OBJDIR)/%.o: %.cpp
	@ mkdir -p $(@D)
	@ $(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf obj
	rm -rf $(ASM_LIB_DIR)
	rm -rf $(ASM_INCLUDE_DIR)
	rm -f $(EXECUTABLE_RELEASE) $(EXECUTABLE_DEBUG)
