CC = gcc
CFLAGS = -Wall -Wextra -Iinclude
LDFLAGS = -lncurses

SRC = src/main.c src/env_detect.c src/cpu_info.c src/memory_info.c src/disk_info.c src/tui.c src/framebuffer.c src/gui.c
OBJ = $(SRC:src/%.c=obj/%.o)
TARGET = build/system_hub

all: build_dir $(TARGET)

build_dir:
	mkdir -p build

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

obj/%.o: src/%.c
	mkdir -p obj
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf obj build

.PHONY: all clean build_dir

