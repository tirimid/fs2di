.PHONY: all

SRC_DIR := src
INC_DIR := include
LIB_DIR := lib

CPP := g++
LD := g++
CPPFLAGS := -std=c++20 -pedantic -I$(INC_DIR)
LDFLAGS :=

SRCS := cli.cc gpt.cc main.cc mbr.cc util.cc
OUT_BIN := fs2di

SRC_PATHS := $(patsubst %,$(SRC_DIR)/%,$(SRCS))
OBJ_PATHS := $(patsubst $(SRC_DIR)/%,$(LIB_DIR)/%.o,$(SRC_PATHS))

all: $(OUT_BIN)

$(OUT_BIN): $(OBJ_PATHS)
	$(LD) $(LDFLAGS) -o $@ $^

$(LIB_DIR)/%.o: $(SRC_DIR)/%
	@ mkdir -p $@
	@ rmdir $@
	$(CPP) $(CPPFLAGS) -o $@ -c $<
