#ifndef INFINITY_FIFOBUF_H
#define INFINTIY_FIFOBUF_H

#include <stdint.h>

#include <infinity/kernel.h>

volatile struct fifo_buffer {
    uint32_t    f_pos;
    uint32_t    f_first;
    uint32_t    f_len;
    uint32_t    f_avail;
    char *      f_buf;    
};

void fifo_init(struct fifo_buffer *buf, int size);
void fifo_read(struct fifo_buffer *buf, void *dest, int n);
void fifo_write(struct fifo_buffer *buf, void *src, int n);

static inline void fifo_writeb(struct fifo_buffer *buf, char c)
{
    while(buf->f_avail >= buf->f_len) asm("hlt");
    uint32_t abspos = buf->f_pos % buf->f_len;
    buf->f_buf[abspos] = c;
    buf->f_pos++;
    buf->f_avail++;
}

static inline char fifo_readb(struct fifo_buffer *buf)
{
    while(buf->f_avail == 0) asm("hlt");
    char ret = buf->f_buf[buf->f_first++];
    buf->f_avail--;
    return ret;
    
}

#endif
