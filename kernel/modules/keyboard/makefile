CC = i686-elf-gcc
AS = nasm
AFLAGS = -f elf32
CFLAGS = -c -O -ffreestanding -nostdlib -std=gnu99 -I ../../include
LD = i686-elf-gcc
LDFLAGS = -O2 -gcc

TARGET = keyboard.ko

OBJECTS = $(patsubst %.c, %.o, $(wildcard *.c) $(shell find ./src -name "*.c")) $(patsubst %.asm, %.o, $(wildcard *.asm) $(shell find ./src -name "*.asm"))

all: $(TARGET)


$(TARGET): $(OBJECTS)
	$(LD) -r -ffreestanding -nostdlib -o $@ $^ -gcc

%.o: %.c
	$(CC) $(CFLAGS) $^ -o $@
%.o: %.asm
	$(AS) $(AFLAGS) $^ -o $@
install:
	cp bin/mkifsrd /usr/local/bin/mkifsrd
clean:
	rm $(TARGET) $(OBJECTS)

