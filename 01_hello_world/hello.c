#define DEBUG
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>

static int __init my_init(void)
{
	pr_info("hello_module - init\n");

	return 0;
}

static void __exit my_exit(void)
{	
	printk("hello_module - exit\n");
}


module_init(my_init);
module_exit(my_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("javago");
MODULE_DESCRIPTION("hello test module");

