CC = i686-elf-gcc
AS = nasm
AFLAGS = -f elf32
CFLAGS = -c -O -ffreestanding -nostdlib -std=gnu99 -I ./include -I "$(INF_CC_INC)"
LD = i686-elf-gcc
LDFLAGS = -O2 -gcc

TARGET = libsys.a

OBJECTS = $(patsubst %.c, %.o, $(wildcard *.c) $(shell find ./src -name "*.c")) $(patsubst %.asm, %.o, $(wildcard *.asm) $(shell find ./src -name "*.asm"))

all: $(TARGET)


$(TARGET): $(OBJECTS)
	$(LD) -r -ffreestanding -nostdlib -o $@ $^ -gcc

%.o: %.c
	$(CC) $(CFLAGS) $^ -o $@
%.o: %.asm
	$(AS) $(AFLAGS) $^ -o $@
	
install:
	cp include/sys/infinity.h $(INF_CC_INC)/sys/infinity.h
	cp libsys.a $(INF_CC_LIB)/libsys.a
clean:
	rm $(TARGET) $(OBJECTS)

