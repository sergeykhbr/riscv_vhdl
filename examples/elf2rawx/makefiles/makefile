include util.mak

CC=gcc
CFLAGS= -c -g -O2
LDFLAGS=
LIBS=-lstdc++

SOURCES = \
	attribute.cpp \
	autobuffer.cpp \
	elfreader.cpp \
	main.cpp
OBJECTS = $(SOURCES:.cpp=.o)
EXECUTABLE = elf2rawx
SRC_PATH = \
	../src \
	../src/common

VPATH = $(SRC_PATH)

OBJ_DIR = obj
ELF_DIR = elf

.PHONY: $(EXECUTABLE)

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(MKDIR) ./$(ELF_DIR)
	$(CC) $(LDFLAGS) $(addprefix $(OBJ_DIR)/,$(OBJECTS)) -o $(addprefix $(ELF_DIR)/,$@) -lstdc++

#.cpp.o:
%.o: %.cpp
	$(MKDIR) ./$(OBJ_DIR)
	$(CC) $(CFLAGS) $< -o $(addprefix $(OBJ_DIR)/,$@)
