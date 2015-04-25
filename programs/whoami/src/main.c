#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <pwd.h>
#include <sys/infinity.h>

int main(char **argv, char **environ)
{
    int uid = sys_getuid();
    struct passwd *pw = getpwuid(uid);
    if(pw) {
        printf("%s\n", pw->pw_name);
    } else {
        printf("whoami: no entry found for uid %d in /etc/passwd!\n", uid);
        sys_exit(-1);
    }
    sys_exit(0);
}
