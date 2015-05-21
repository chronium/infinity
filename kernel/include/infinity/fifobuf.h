#ifndef INFINITY_FIFOBUF_H
#define INFINTIY_FIFOBUF_H

#include <stdint.h>

#include <infinity/kernel.h>

volatile struct fifo_buffer {
    uint32_t    f_pos;
    uint32_t    f_first;
    uint32_t    f_len;
    uint32_t    f_avail;
    char        f_brk;
    char *      f_buf;    
};

void fifo_init(struct fifo_buffer *buf, int size);
int fifo_read(struct fifo_buffer *buf, void *dest, int n);
int fifo_write(struct fifo_buffer *buf, void *src, int n);

static inline void fifo_writeb(struct fifo_buffer *buf, char c)
{
    if(buf->f_avail == 0) {
        buf->f_pos = 0;
        buf->f_first = 0;
    }
    while(buf->f_avail >= buf->f_len && buf->f_brk == 0) asm("hlt");
    if(buf->f_brk) return;
    uint32_t abspos = buf->f_pos % buf->f_len;
    buf->f_buf[abspos] = c;
    buf->f_pos++;
    buf->f_avail++;
}

static inline char fifo_readb(struct fifo_buffer *buf)
{
    while(buf->f_avail == 0 && buf->f_brk == 0) asm("hlt");
    if(buf->f_brk && buf->f_avail == 0) return 0;
    char ret = buf->f_buf[buf->f_first++];
    buf->f_avail--;
    return ret;
    
}

#endif
