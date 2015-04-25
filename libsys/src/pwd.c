#include <pwd.h>
#include <fcntl.h>
#include <sys/infinity.h>

struct passwd_buf {
    struct passwd   b_passwd;
    char            b_name[32];
    char            b_pass[64];
    char            b_dir[64];
    char            b_shell[64];
};


static int read_passwd_u(uid_t uid, struct passwd_buf *buf);
static int read_passwd_n(const char *username, struct passwd_buf *buf);
static int read_field(int fd, char *buf);
static int skip_line(int fd);

static struct passwd_buf passwd_storage;
/*
 * This library shouldn't be dependent on stdlib, hence the implementation
 * of the following standard functions
 */
static int strcmp(const char *str1, const char *str2);
static int strncmp(const char *str1, const char *str2, size_t len);
static int atoi(char *s);
static void reverse(char *str);
static int atoi(char *s);

struct passwd *getpwuid(uid_t uid)
{
    if(read_passwd_u(uid, &passwd_storage) == 0) {
        struct passwd *ret = &passwd_storage.b_passwd;
        ret->pw_name = passwd_storage.b_name;
        ret->pw_shell = passwd_storage.b_shell;
        ret->pw_dir = passwd_storage.b_dir;
        ret->pw_passwd = passwd_storage.b_pass;
        return ret;
    } else {
        return 0;
    }
}

struct passwd *getpwnam(const char *uname)
{
    if(read_passwd_n(uname, &passwd_storage) == 0) {
        struct passwd *ret = &passwd_storage.b_passwd;
        ret->pw_name = passwd_storage.b_name;
        ret->pw_shell = passwd_storage.b_shell;
        ret->pw_dir = passwd_storage.b_dir;
        ret->pw_passwd = passwd_storage.b_pass;
        return ret;
    } else {
        return 0;
    }
}

static inline int strlen(const char *str)
{
    int i = 0;
    while (str[i] != 0) i++;
    return i;
}    

static int strncmp(const char *str1, const char *str2, size_t len)
{
    for (int i = 0; i < len; i++)
        if (str1[i] != str2[i])
            return -1;
    return 0;
}

static int strcmp(const char *str1, const char *str2)
{
    if (strlen(str1) > strlen(str2))
        return strncmp(str1, str2, strlen(str1));
    else
        return strncmp(str1, str2, strlen(str2));
}

static int atoi(char *s)
{
    int final = 0;
    int mul = 1;
    int len = strlen(s);
    reverse(s);
    for (int i = 0; i < len; i++) {
        char b = s[i];
        int RealDigit = (int)b - 48;
        final = (RealDigit * mul) + final;
        mul = mul * 10;
    }
    return final;
}

static void reverse(char *str)
{
    int start = 0;
    int end = strlen(str) - 1;

    while (start < end) {
        char c1 = *(str + start);
        char c2 = *(str + end);
        *(str + start) = c2;
        *(str + end) = c1;
        start++;
        end--;
    }
}

static int read_passwd_u(uid_t _uid, struct passwd_buf *buf)
{
    int fd = sys_open("/etc/passwd", O_RDWR);
    
    char line[512];
    char uname[64];
    char pass[64];
    char uid[64];
    char gid[64];
    char name[64];
    char home[64];
    char shell[64];
    
    while (read_field(fd, buf->b_name)) {
        
        read_field(fd, buf->b_pass);
        read_field(fd, uid);
        read_field(fd, gid);
        
        buf->b_passwd.pw_uid = atoi(uid);
        buf->b_passwd.pw_gid = atoi(gid);
        
        if(_uid == buf->b_passwd.pw_uid) {
            read_field(fd, name);
            read_field(fd, buf->b_dir);
            read_field(fd, buf->b_shell);
            sys_close(fd);
            return 0;
        }
        skip_line(fd);
    }
    sys_close(fd);
    return -1;
}

static int read_passwd_n(const char *username, struct passwd_buf *buf)
{
    int fd = sys_open("/etc/passwd", O_RDWR);
    
    char line[512];
    char uname[64];
    char pass[64];
    char uid[64];
    char gid[64];
    char name[64];
    char home[64];
    char shell[64];
    
    while (read_field(fd, uname)) {
        if(strcmp(uname, username) == 0) {
            
            read_field(fd, buf->b_pass);
            read_field(fd, uid);
            read_field(fd, gid);
            read_field(fd, buf->b_name);
            read_field(fd, buf->b_dir);
            read_field(fd, buf->b_shell);
            buf->b_passwd.pw_uid = atoi(uid);
            buf->b_passwd.pw_gid = atoi(gid);
            sys_close(fd);
            return 0;
        }
        skip_line(fd);
    }
    sys_close(fd);
    return -1;
}

static int skip_line(int fd)
{
    int i = 0;
    char dat = 0;
    while(sys_read(fd, &dat, 1) && dat && dat != '\n') {
        i++;
    }
    return i;
}

static int read_field(int fd, char *buf)
{
    int i = 0;
    char dat = 0;
    while(sys_read(fd, &dat, 1) && dat && dat != ':' && dat != '\n') {
        buf[i++] = dat;
    }
    buf[i] = 0;
    return i;
}
