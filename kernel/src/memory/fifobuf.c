#include <infinity/fifobuf.h>
#include <infinity/heap.h>

void fifo_init(struct fifo_buffer *buf, int size)
{
    buf->f_len = size;
    buf->f_buf = kalloc(size);
    buf->f_first = 0;
    buf->f_pos = 0;
    buf->f_avail = 0;
}

int fifo_read(struct fifo_buffer *buf, void *dest, int n)
{
    for(int i = 0; i < n; i++) {
        *((char*)dest + i) = fifo_readb(buf);
        if(buf->f_brk && buf->f_avail == 0)
            return -1;
    }
    return n;
}

int fifo_write(struct fifo_buffer *buf, void *src, int n)
{
    for(int i = 0; i < n; i++) {
        fifo_writeb(buf, *((char*)src + i));
        if(buf->f_brk)
            return -1;
    }
    return n;
}
