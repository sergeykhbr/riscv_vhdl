include util.mak

CC=gcc
CFLAGS= -c -g -O2
LDFLAGS=
LIBS=-lstdc++

SOURCES = \
	elfreader.cpp \
	main.cpp
OBJECTS = $(SOURCES:.cpp=.o)
EXECUTABLE = elf2raw64
SRC_DIR = ../src
OBJ_DIR = obj
ELF_DIR = elf

.PHONY: $(EXECUTABLE)

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(MKDIR) ./$(ELF_DIR)
	$(CC) $(LDFLAGS) $(addprefix $(OBJ_DIR)/,$(OBJECTS)) -o $(addprefix $(ELF_DIR)/,$@) -lstdc++

#.cpp.o:
%.o: $(SRC_DIR)/%.cpp
	$(MKDIR) ./$(OBJ_DIR)
	$(CC) $(CFLAGS) $< -o $(addprefix $(OBJ_DIR)/,$@)
