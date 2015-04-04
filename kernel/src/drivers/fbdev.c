#include <stdint.h>
#include <mboot.h>
#include <infinity/heap.h>
#include <infinity/kernel.h>
#include <infinity/device.h>
#include <infinity/paging.h>
#include <infinity/drivers/serial.h>
#include <infinity/drivers/framebuffer.h>

struct device *fb_dev;

static size_t fb_write(void *tag, const char *data, size_t s, uint32_t add);
static size_t fb_read(void *tag, const char *data, size_t s, uint32_t add);
 
void init_fb(vbe_info_t *info)
{
    struct fb_info *tag = (struct fb_info*)kalloc(sizeof(struct fb_info));
    tag->res_x = info->Xres;
    tag->res_y = info->Yres;
    tag->pitch = info->pitch;
    tag->depth = info->pitch / info->Xres;
    tag->frame_buffer = (char*)info->physbase;
    tag->frame_buffer_length = info->Yres * info->pitch;
    fb_dev = device_create(CHAR_DEVICE, "framebuffer");
    fb_dev->write = fb_write;
    fb_dev->read = fb_read;
    fb_dev->dev_tag = tag;
    extern struct page_directory *current_directory;
    for(int i = 0; i < tag->frame_buffer_length; i += 0x1000) {
        page_alloc(current_directory, tag->frame_buffer + i, tag->frame_buffer + i, 1, 1);
    }
    
    init_fbtty(tag);
}
 
static size_t fb_write(void *tag, const char *data, size_t size, uint32_t addr)
{
    struct fb_info *info = (struct fb_info*)tag;
    int i;
    for(i = 0; i < size; i++) {
        int p = (addr + i) % info->frame_buffer_length;
        info->frame_buffer[p] = ((char*)data)[i];
    }
    return i;
}
 
static size_t fb_read(void *tag, const char *data, size_t size, uint32_t addr)
{
        struct fb_info *info = (struct fb_info*)tag;
        int i;
        for(i = 0; i < size; i++) {
                int p = (addr + i) % info->frame_buffer_length;
                *((char*)data + i) = info->frame_buffer[p];
        }
        return i;
}
