# Имя компилятора
CC = gcc

# Флаги компилятора
CFLAGS = -Wall -Wextra -O2

# Исходные файлы
SRC = entries_tools.c syscalls.c main.c

# Объектные файлы
OBJ = $(SRC:.c=.o)

# Имя исполняемого файла
OUTPUT = fat32Emulator

# Правило по умолчанию
all: $(OUTPUT)

# Правило сборки исполняемого файла
$(OUTPUT): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

# Правило сборки объектных файлов
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Очистка
clean:
	rm -f $(OBJ) $(OUTPUT)

# Файлы не являющиеся исходными
.PHONY: all clean
