#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <dirent.h>

int main(char **argv) {
	open("/dev/tty0", O_RDWR);
	open("/dev/tty0", O_RDWR);
	struct DIR *dp = opendir("/");
	struct dirent *de = readdir(dp);
	printf("Args are %s\n", argv[0]);
	printf("Is at %x\n", argv[0]);
	printf("Hello, World!\n");
	char tmp[100];
	gets(tmp);
	printf("You typed %s\n", tmp);
	exit(0);
}
