CC = gcc
CFLAGS = -Wall -Wextra
LIBS = -lncurses
TARGET = systeminfo
PREFIX = /usr/local

all: $(TARGET)

$(TARGET): main.c
	$(CC) $(CFLAGS) -o $(TARGET) main.c $(LIBS)

install: $(TARGET)
	install -m 0755 $(TARGET) $(PREFIX)/bin/$(TARGET)

uninstall:
	rm -f $(PREFIX)/bin/$(TARGET)

clean:
	rm -f $(TARGET)

.PHONY: all install uninstall clean
