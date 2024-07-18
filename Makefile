CC = gcc

CFLAGS = -Wall -Wextra -O2

SRC = entries_tools.c syscalls.c main.c

OBJ = $(SRC:%.c=build/%.o)

OUTPUT = build/fat32

all: $(OUTPUT)

build:
	mkdir -p build

$(OUTPUT): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

build/%.o: %.c | build
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf build
	
.PHONY: all clean build
