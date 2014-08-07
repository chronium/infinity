CC = i686-elf-gcc
AS = nasm
AFLAGS = -f elf32
CFLAGS = -c -std=gnu99 -ffreestanding -O0 -Wall -w -g -I ./include
LD = $(CC)
LDFLAGS = -T linker.ld -ffreestanding -O2 -nostdlib -gcc

TARGET = bin/infinity.bin

OBJECTS = $(patsubst %.c, %.o, $(wildcard *.c) $(shell find ./src -name "*.c")) $(patsubst %.asm, %.o, $(wildcard *.asm) $(shell find ./src -name "*.asm"))

all: $(TARGET)


$(TARGET): $(OBJECTS)
	$(LD) -o $@ -T linker.ld -ffreestanding -O2 -nostdlib $^ -gcc

%.o: %.c
	$(CC) $(CFLAGS) $^ -o $@
%.o: %.asm
	$(AS) $(AFLAGS) $^ -o $@
clean:
	rm $(TARGET) $(OBJECTS)

iso:
	nm bin/infinity.bin > bin/initrd/boot/INFINITY.MAP
	mkinfrd -d bin/initrd -o bin/iso/initrd.img
	cp bin/infinity.bin bin/iso/boot/infinity.bin
	cp config/grub.cfg bin/iso/boot/grub/grub.cfg
	grub2-mkrescue -o bin/infinity.iso bin/iso
