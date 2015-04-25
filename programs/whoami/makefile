CC = i686-elf-gcc
AS = nasm
AFLAGS = -f elf32
CFLAGS = -c -std=gnu99 -I "$(INF_CC_INC)"
LD = i686-elf-ld
LDFLAGS = -O2 -gcc

TARGET = whoami

OBJECTS = $(patsubst %.c, %.o, $(wildcard *.c) $(shell find ./src -name "*.c")) $(patsubst %.asm, %.o, $(wildcard *.asm) $(shell find ./src -name "*.asm"))



all: $(TARGET)

$(TARGET): $(OBJECTS)
	if ! [ -f lib/libc.a ]; then \
		cp $(INF_CC_LIB)/libc.a lib/libc.a; \
	fi
	
	if ! [ -f lib/libsys.a ]; then \
		cp $(INF_CC_LIB)/libsys.a lib/libsys.a; \
	fi
	
	if ! [ -f lib/libnosys.a ]; then \
		cp $(INF_CC_LIB)/libnosys.a lib/libnosys.a; \
	fi
	
	$(LD) -o $@ $^ ./lib/libc.a ./lib/libnosys.a ./lib/libsys.a -e main

%.o: %.c
	$(CC) $(CFLAGS) $^ -o $@
%.o: %.asm
	$(AS) $(AFLAGS) $^ -o $@

install:
	cp $(TARGET) $(INF_BIN_PATH)/$(TARGET)
clean:
	rm $(TARGET) $(OBJECTS)

