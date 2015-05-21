#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/infinity.h>

static int match_pattern(const char *line, const char *pattern);
static void search_file(int fd, const char *pattern);
static int read_line(int fd, int len, char *buf);

int main(char **argv, char **environ)
{
    search_file(0, argv[1]);
    sys_exit(0);
}

static void search_file(int fd, const char *pattern)
{
    char c = NULL;
    int len = strlen(pattern);
    
    char line[1024];
    while(read_line(fd, 1024, line)) {
        if(match_pattern(line, pattern)) {
            printf("%s\n", line);
        }
    }
}

static int match_pattern(const char *line, const char *pattern)
{
    int len = strlen(line);
    int plen = strlen(pattern);
    int mpos = 0;
    for(int i = 0; i < len; i++) {
        if(line[i] == pattern[mpos]) {
            mpos++;
            if(mpos == plen)
                return 1;
        } else {
            mpos = 0;
        }
    }
    return 0;
}

static int read_line(int fd, int len, char *buf)
{
    int i = 0;
    char dat = 0;
    int r;
    while((r = read(fd, &dat, 1)) && dat && dat != '\n' && i < len) {
        buf[i++] = dat;
    }
    buf[i] = 0;
    return r > 0;
}
