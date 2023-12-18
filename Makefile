# sources: https://stackoverflow.com/questions/30573481/how-to-write-a-makefile-with-separate-source-and-header-directories

#CC ?= gcc
CC ?= g++

SRC_DIR := src
INC_DIR := include $(shell pkg-config --cflags opencv4)
OBJ_DIR := obj
BIN_DIR := bin

EXE           := $(BIN_DIR)/object-detection
SRC           := $(wildcard $(SRC_DIR)/*.cpp)
OBJ           := $(SRC:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)

CPPFLAGS += -I$(INC_DIR) -MMD -MP
CFLAGS   += -g -Wall -Wextra -std=c++17
LDLIBS   += -lstdc++ -lgphoto2 $(shell pkg-config --libs opencv4)

# Define VERSION_HASH (git sha1) and VERSION_DATE (build date)
$(eval DEF += -DVERSION_HASH='"$(shell git describe --dirty --always)"')
$(eval DEF += -DVERSION_DATE='"$(shell date +"%Y-%m-%d %H:%M:%S")"')

.PHONY: all clean

all: $(EXE)

$(EXE): $(OBJ) | $(BIN_DIR)
	$(CC) $(LDFLAGS) $^ -o $@ $(LDLIBS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DEF) -c $< -o $@

$(BIN_DIR) $(OBJ_DIR) $(GEN_INC_DIR):
	mkdir -p $@

clean:
	@echo "cleaning ..."
	@$(RM) -rv $(BIN_DIR) $(OBJ_DIR)

-include $(OBJ:.o=.d)