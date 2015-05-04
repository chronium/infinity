#include <infinity/heap.h>
#include <infinity/fs.h>
#include <infinity/fifobuf.h>

static int named_pipe_write(struct file *fd, const char *buf, off_t off, size_t len);
static int named_pipe_read(struct file *fd, char *buf, off_t off, size_t len);

void named_pipe_open(struct file *f)
{
    struct fifo_buffer *buf = (struct fifo_buffer*)kalloc(sizeof(struct fifo_buffer));
    fifo_init(buf, 4096);
    f->f_tag = buf;
    f->write = named_pipe_write;
    f->read = named_pipe_read;
}

static int named_pipe_write(struct file *fd, const char *buf, off_t off, size_t len)
{
    struct fifo_buffer *pipe = fd->f_tag;
    printk(KERN_INFO "Fifo write! %s %d (Pipe %x)\n", buf, len, pipe);
    fifo_write(pipe, buf, len);
    return len;
}

static int named_pipe_read(struct file *fd, char *buf, off_t off, size_t len)
{
    struct fifo_buffer *pipe = fd->f_tag;
    fifo_read(pipe, buf, len);
    return len;
}

