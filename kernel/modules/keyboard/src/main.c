#include <infinity/kernel.h>
#include <infinity/arch/pic.h>
#include <infinity/arch/portio.h>
#include <infinity/device.h>
#include <infinity/tty.h>

#define PS2_DATA    0x60
#define PS2_CTL     0x64

const uint8_t keymap_en[128] = {
  0,                        //               0
  27,                       // ESC ^[        1
  '1', '2', '3', '4', '5',  // Numbers       2 -  6
  '6', '7', '8', '9', '0',  // Numbers       7 - 11
  '-', '=', '\b', '\t',     //              12 - 15
  'q', 'w', 'e', 'r', 't',  // Chars        16 - 20
  'y', 'u', 'i', 'o', 'p',  // Chars        21 - 25
  '[', ']', '\n',           //              26 - 28
  0,                        // Control      29
  'a', 's', 'd', 'f', 'g',  // Chars        30 - 34
  'h', 'j', 'k', 'l',       // Chars        35 - 38
  ';', '\'', '`',           //              39 - 41
  0,                        // Lshift       42
  '\\',                     //              43
  'z', 'x', 'c', 'v', 'b',  // Chars        44 - 48
  'n', 'm',                 // Chars        49 - 50
  ',', '.', '/',            //              51 - 53
  0,                        // Rshift       54
  '*',                      //              55
  0,                        // Alt          56
  ' ',                      // Space        57
  0,                        // Caps lock    58
  0, 0, 0, 0, 0,            // F1 - F5      59 - 63
  0, 0, 0, 0, 0,            // F6 - F10     64 - 68
  0,                        // Num lock     69
  0,                        // Scroll lock  70
  0,                        // Home key     71
  0,                        // Arrow up     72
  0,                        // Page up      73
  '-',                      // Num minus    74
  0,                        // Arrow left   75
  0,                        //              76
  0,                        // Arrow right  77
  '+',                      // Num plus     78
  0,                        // End          79
  0,                        // Arrow down   80
  0,                        // Page down    81
  0,                        // Insert       82
  0,                        // Delete       83
  0, 0, 0,                  //              84 - 87
  0, 0,                     // F11 - F12    88 - 89
  0,                        //              90
};

const uint8_t keymap_shift_en[128] = {
  0,                        //               0
  27,                       // ESC ^[        1
  '!', '@', '#', '$', '%',  // Numbers       2 -  6
  '^', '&', '*', '(', ')',  // Numbers       7 - 11
  '_', '+', '\b', '\t',     //              12 - 15
  'Q', 'W', 'E', 'R', 'T',  // Chars        16 - 20
  'Y', 'U', 'I', 'O', 'P',  // Chars        21 - 25
  '{', '}', '\n',           //              26 - 28
  0,                        // Control      29
  'A', 'S', 'D', 'F', 'G',  // Chars        30 - 34
  'H', 'J', 'K', 'L',       // Chars        35 - 38
  ':', '\"', '`',           //              39 - 41
  0,                        // Lshift       42
  '|',                     //              43
  'Z', 'X', 'C', 'V', 'B',  // Chars        44 - 48
  'N', 'M',                 // Chars        49 - 50
  '<', '>', '?',            //              51 - 53
  0,                        // Rshift       54
  '*',                      //              55
  0,                        // Alt          56
  ' ',                      // Space        57
  0,                        // Caps lock    58
  0, 0, 0, 0, 0,            // F1 - F5      59 - 63
  0, 0, 0, 0, 0,            // F6 - F10     64 - 68
  0,                        // Num lock     69
  0,                        // Scroll lock  70
  0,                        // Home key     71
  0,                        // Arrow up     72
  0,                        // Page up      73
  '-',                      // Num minus    74
  0,                        // Arrow left   75
  0,                        //              76
  0,                        // Arrow right  77
  '+',                      // Num plus     78
  0,                        // End          79
  0,                        // Arrow down   80
  0,                        // Page down    81
  0,                        // Insert       82
  0,                        // Delete       83
  0, 0, 0,                  //              84 - 87
  0, 0,                     // F11 - F12    88 - 89
  0,                        //              90
};


static int kb_shift = 0;
static volatile int kb_ack = 0;
static struct device *kb_dev;

static int kb_self_test();
static void kb_clear();
static void kb_send_command(char cmd);
static void kb_irq_handler(struct regs *r);
static void kb_wait();
static size_t kb_read(void *tag, void *buff, int off, size_t size);

int mod_init()
{
    if(request_irq(2, kb_irq_handler) == 0) {
       
        kb_send_command(0xED);
        kb_send_command(0x00);
        kb_send_command(0xF3);
        kb_send_command(0x00);
        kb_send_command(0xF4);
        printk(KERN_INFO "ps2_kb: keyboard initialized\n");
        
        
    } else {
        printk(KERN_ERR "ps2_kb: Could not install IRQ handler!\n");
        return -1;
    }
	return 0;
}


int mod_uninit()
{
	return 0;
}

static int kb_self_test()
{
    outb(PS2_CTL, 0xAA);
    return inb(PS2_DATA) == 0x55;
}

static void kb_clear()
{
    while(inb(PS2_CTL) & 0x1)
        inb(PS2_DATA);
}

static void kb_send_command(char cmd)
{
    while(inb(PS2_CTL) & 0x2);
    outb(PS2_DATA, cmd);
}


static void kb_irq_handler(struct regs *r)
{
    while(inb(PS2_CTL) & 0x1) {
        uint8_t scancode = inb(PS2_DATA);
        if (scancode & 0x80) {
            uint8_t msc = scancode & ~0x80;
            if(msc == 42 || msc == 54)
                kb_shift = 0;
        } else if (scancode == 42 || scancode == 54) {
          kb_shift = 1;  
        } else {
            char c = kb_shift ? keymap_shift_en[scancode] : keymap_en[scancode];
            if(c) {
                tty_writec(c);
            }
            kb_ack = 1;
        }
    }
}

static void kb_wait()
{
    kb_ack = 0;
    while(!kb_ack) asm("hlt");
}

static size_t kb_read(void *tag, void *buff, int off, size_t size)
{
	//memcpy(buff, &initrd_ptr[off], size);
	return size;
}

