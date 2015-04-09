

int open(const char *path, int mode) 
{
	int r;
	asm volatile ( "int $0x80" : "=a" (r) : "a" (5), "b" (path), "c" (mode));
	return r;
}

int write(int fd, const char *buf, int n)
{
	int r;
	asm volatile ("int $0x80" : "=a" (r) : "a" (4), "b" (fd), "c" (buf), "d" (n));
	return r;
}

int sleep(int ticks)
{
	for(int i = 0; i < ticks; i++) {
		asm volatile ("hlt");
	}
}

int main() {
	int fd = open("/dev/tty0", 0xFF);
	while(1) {
		write(fd, "Hello, World!", 13);
		sleep(50);
	}
	return 1013;
}
