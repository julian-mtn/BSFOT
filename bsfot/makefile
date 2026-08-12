CC = gcc

TARGET = main

# ============================================================
# RELIC curve selection
# ============================================================

# Example:
#   make CURVE=bn254
#
CURVE ?= bn254

RELIC_DIR = /opt/relic-$(CURVE)

# ============================================================
# Message size
# ============================================================

# Example:
#   make CURVE=bn254 MESSAGE_BITS=128
#
MESSAGE_BITS ?= 128

# ============================================================
# Compilation flags
# ============================================================

CFLAGS = -Wall -Wextra -O2 \
         -Iinclude \
         -I$(RELIC_DIR)/include \
         -DMESSAGE_BITS=$(MESSAGE_BITS)

# ============================================================
# Linker flags
# ============================================================

LDFLAGS = -L$(RELIC_DIR)/lib

LIBS = -lrelic_s -lgmp -lm

# ============================================================
# Source files
# ============================================================

SRC = $(shell find src -name "*.c") main.c

# Object files

OBJ = $(SRC:src/%.c=build/%.o)
OBJ := $(OBJ:main.c=build/main.o)

# ============================================================
# Build
# ============================================================

all: $(TARGET)

# Linking

$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $(TARGET) $(LDFLAGS) $(LIBS)

# Compilation of source files

build/%.o: src/%.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# Compilation of main.c

build/main.o: main.c
	mkdir -p build
	$(CC) $(CFLAGS) -c $< -o $@

# ============================================================
# Run program
# ============================================================

run: $(TARGET)
	./$(TARGET) $(MODE) $(MESSAGE_BITS)

# ============================================================
# Clean generated files
# ============================================================

clean:
	rm -rf build $(TARGET)

# ============================================================
# Generate compile_commands.json
# ============================================================

bear: clean
	bear -- make

.PHONY: all run clean bear