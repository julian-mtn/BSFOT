CC = gcc

TARGET = main

CFLAGS = -Wall -Wextra -O2 -Iinclude

LIBS = -lrelic -lgmp -lm


SRC = $(shell find src -name "*.c") main.c

OBJ = $(SRC:src/%.c=build/%.o)
OBJ := $(OBJ:main.c=build/main.o)


all: $(TARGET)


$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $(TARGET) $(LIBS)


build/%.o: src/%.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@


build/main.o: main.c
	mkdir -p build
	$(CC) $(CFLAGS) -c $< -o $@


run: $(TARGET)
	./$(TARGET)


clean:
	rm -rf build $(TARGET)


bear: clean
	bear -- make


.PHONY: all run clean bear
