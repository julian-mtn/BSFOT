CC = gcc

TARGET = main

# Choix de la courbe RELIC
# Exemple : make CURVE=bn254
CURVE ?= bn254

RELIC_DIR = /opt/relic-$(CURVE)

CFLAGS = -Wall -Wextra -O2 \
         -Iinclude \
         -I$(RELIC_DIR)/include

LDFLAGS = -L$(RELIC_DIR)/lib

LIBS = -lrelic_s -lgmp -lm


SRC = $(shell find src -name "*.c") main.c

OBJ = $(SRC:src/%.c=build/%.o)
OBJ := $(OBJ:main.c=build/main.o)


all: $(TARGET)


$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $(TARGET) $(LDFLAGS) $(LIBS)


build/%.o: src/%.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@


build/main.o: main.c
	mkdir -p build
	$(CC) $(CFLAGS) -c $< -o $@


run: $(TARGET)
	./$(TARGET) $(MODE)


clean:
	rm -rf build $(TARGET)


bear: clean
	bear -- make


.PHONY: all run clean bear