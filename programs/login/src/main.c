#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <pwd.h>
#include <sys/infinity.h>
#include "sha1.h"

static int read_passwd(const char *username, struct passwd *buf);
static int skip_line(int fd);
static int read_field(int fd, char *buf);
static void parse_bytes(char *res, char *hash);
static void logon(const char *password, struct passwd *logon);

int main(char **argc, char **environ)
{
    char username[32];
    char password[32];
    struct passwd *pw;
    while(1) {
        printf("\nPlease login: ");
        fflush(stdout);
        gets(username);
        fflush(stdin);
        printf("Password: \x1B[8m");
        fflush(stdout);
        gets(password);
        printf("\x1B[0m");
        fflush(stdin);
        pw = getpwnam(username);
        if(pw) {
            logon(password, pw);
        } else {
            printf("Login incorrect\n");
        }
        memset(username, 0, 32);
        memset(password, 0, 32);
    }
}

static void logon(const char *password, struct passwd *logon)
{
    sha1_ctx_t ctx;
    char hashed[20];
    sha1_begin(&ctx);
    sha1_hash(password, strlen(password), &ctx);
    sha1_end(hashed, &ctx);
    char hashed_pw[20];
    parse_bytes(hashed_pw, logon->pw_passwd);
    if(memcmp(hashed, hashed_pw, 20) == 0) {
        printf("\x1B\x63");
        fflush(stdout);
        if(sys_setuid(logon->pw_uid) == 0) {
            sys_spawnve(P_WAIT, logon->pw_shell, NULL, NULL);
            sys_setwd(logon->pw_dir);
        } else {
            printf("Login - could not set uid! Are you running this as an unprivileged user?\n");
        }
    } else {
        printf("Login incorrect\n");
    }
}

static int skip_line(int fd)
{
    int i = 0;
    char dat = 0;
    while(read(fd, &dat, 1) && dat && dat != '\n') {
        i++;
    }
    return i;
}

static int read_field(int fd, char *buf)
{
    int i = 0;
    char dat = 0;
    while(read(fd, &dat, 1) && dat && dat != ':' && dat != '\n') {
        buf[i++] = dat;
    }
    buf[i] = 0;
    return i;
}

static void parse_bytes(char *buf, char *hash)
{
    int len = strlen(hash);
    char tmp[3];
    tmp[2] = 0;
    int j = 0;
    for(int i = 0; i < len; i += 2) {
        memcpy(tmp, &hash[i], 2);
        int res = strtol(tmp, NULL, 16);
        buf[j++] = res;
    }
}
