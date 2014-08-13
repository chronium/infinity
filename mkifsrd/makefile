CC = gcc
AS = nasm
AFLAGS = -f elf32
CFLAGS = -c -std=gnu99
LD = gcc
LDFLAGS = -O2 -gcc

TARGET = bin/mkifsrd

OBJECTS = $(patsubst %.c, %.o, $(wildcard *.c) $(shell find ./src -name "*.c")) $(patsubst %.asm, %.o, $(wildcard *.asm) $(shell find ./src -name "*.asm"))

all: $(TARGET)


$(TARGET): $(OBJECTS)
	$(LD) -o $@ $^ -gcc

%.o: %.c
	$(CC) $(CFLAGS) $^ -o $@
%.o: %.asm
	$(AS) $(AFLAGS) $^ -o $@
install:
	cp bin/mkifsrd /usr/local/bin/mkifsrd
clean:
	rm $(TARGET) $(OBJECTS)

