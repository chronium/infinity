#include <infinity/kernel.h>


int mod_init()
{
	printk(KERN_INFO "Hello, infinity kernel!\n");
	return 0;
}

int mod_uninit()
{
	printk(KERN_INFO "Goodbye infinity kernel :(\n");
}
