#define DEBUG
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/ktime.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/uaccess.h>
#include <linux/version.h>

#define PROC_NAME "utc-now"
#define BUFFER_SIZE 256

ktime_t start;
int utc_offset_hours = 9;

static int format_utc(char *buf, size_t size)
{
	struct timespec64 now;
	struct tm local;
	size_t len;

	if (utc_offset_hours < -12 || utc_offset_hours > 14)
		return -EINVAL;
	
	ktime_get_real_ts64(&now);
	time64_to_tm(now.tv_sec, utc_offset_hours * 3600, &local);

	len = snprintf(buf, size, "%04ld-%02d-%02d %02d:%02d:%02d",
			local.tm_year + 1900, local.tm_mon + 1,
			local.tm_mday, local.tm_hour, local.tm_min, local.tm_sec);

	if (len >= size)
		return -ENOSPC;
	
	return len;
}

static int handle_proc_show(struct seq_file *m, void *v){
	char utc_str[32];
	
	int ret = format_utc(utc_str, sizeof(utc_str));
	if (ret < 0)
		return ret;
	seq_printf(m, "UTC%+d %s\n", utc_offset_hours, utc_str);

	return 0;
}

static int handle_proc_open(struct inode *inode, struct file *file){
	return single_open(file, handle_proc_show, NULL);
}

static ssize_t handle_proc_write(
		struct file *file, 
		const char __user *user_buf, 
		size_t count, loff_t *offset)
{
	/*
	size_t len = min(count, (size_t)(BUFFER_SIZE - 1));

	if (copy_from_user(proc_buffer, user_buf, len))
		return -EFAULT;

	proc_buffer[len] = '\0';
	proc_buffer_len = len;
	if (len > 0 && proc_buffer[len - 1] == '\n') {
		proc_buffer[len - 1] = '\0';
		proc_buffer_len--;
	}
	*/
	
	int ret = kstrtoint_from_user(user_buf, count, 10, &utc_offset_hours);
	if (ret) {
		pr_err("set UTC offset error\n");
		return ret;
	}
	else {
		pr_info("set UTC offset to %d\n", utc_offset_hours);
	}

	return count;
}

static const struct proc_ops hello_proc_ops = {
	.proc_open = handle_proc_open,
	.proc_read = seq_read,
	.proc_write = handle_proc_write,
	.proc_lseek = seq_lseek,
	.proc_release = single_release,
};

static struct proc_dir_entry *proc_entry;

static int __init my_init(void)
{
	pr_info("utc-now module - init\n");
	start = ktime_get();

	proc_entry = proc_create(PROC_NAME, 0644, NULL, &hello_proc_ops);
	if(!proc_entry) {
		pr_err("/proc/%s\n", PROC_NAME);
		return -ENOMEM;
	}
	pr_info("/proc/%s 생성 완료\n", PROC_NAME);

	return 0;
}

static void __exit my_exit(void)
{
	proc_remove(proc_entry);
	pr_info("/proc/%s 제거 완료\n", PROC_NAME);
	
	ktime_t end;
	end = ktime_get();
	printk("module run time: %lldms\n", ktime_ms_delta(end, start));
	
	printk("utc-now module - exit\n");
}


module_init(my_init);
module_exit(my_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("javago");
MODULE_DESCRIPTION("hello test module");

