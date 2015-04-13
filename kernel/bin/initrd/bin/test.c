#include <stdlib.h>
#include <string.h>

extern int open(const char *path, int mode);
extern int write(int fd, char *ptr, int n);

static int sleep(int delay) {
	for(int i = 0; i < delay; i++) {
		asm("hlt");
	}
}

int main() {

	malloc(0xFFFF);	
	int fd = open("/dev/tty0", 0xFF);
	int col = 0;
	char * colors[] = {"\x1B[31m", "\x1B[32m", "\x1B[33m", "\x1B[c34m"
			   "\x1B[35m", "\x1B[36m", "\x1B[37m"};
	while(1) {
		write(fd, "\x1B[s\x1BH", 5);
		write(fd, colors[col++], 5);
		write(fd, "Hello, World!", 13);
		write(fd, "\x1B[u", 3);
		write(fd, "\x1B[0m", 4);

		sleep(200);
		
		if(col > 4)
			col = 0;
	}
	return 1013;
}
