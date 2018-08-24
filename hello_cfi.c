#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/mm.h>
#include <linux/highmem.h>
#include <linux/io.h>
#include <linux/types.h>
#include <linux/slab.h>

#include <linux/kallsyms.h>

typedef int (*int_arg_fn)(int);
typedef int (*float_arg_fn)(float);

static int int_arg(int arg) {
    printk("In %s: (%d)\n", __FUNCTION__, arg);
    return 0;
}

static int float_arg(float arg) {
    printk("CFI should protect transfer to here\n");
    printk("In %s: (%d)\n", __FUNCTION__, (int)arg);
    return 0;
}

static int bad_int_arg(int arg) {
    printk("CFI will not protect transfer to here\n");
    printk("In %s: (%d)\n", __FUNCTION__, arg);
    return 0;
}

extern void not_entry_point(void);

struct foo {
    int_arg_fn int_funcs[1];
    int_arg_fn bad_int_funcs[1];
    float_arg_fn float_funcs[1];  
    int_arg_fn not_entry_begin[1];
    int_arg_fn not_entry_middle[1];
};

// the struct aligns the function pointer arrays
// so indexing past the end will reliably
// call working function pointers
static struct foo f = { 
    .int_funcs = {int_arg},
    .bad_int_funcs = {bad_int_arg},
    .float_funcs = {float_arg},
    .not_entry_begin = {(int_arg_fn)((uintptr_t)(not_entry_point))},
    .not_entry_middle = {(int_arg_fn)((uintptr_t)(not_entry_point)+0x20)}
};

static int hello_is_prefix(const char * prefix, const char * string) 
{
	return strncmp(prefix, string, strlen(prefix)) == 0;
}

static ssize_t hello_write(struct file *file, const char __user *buf,
				size_t datalen, loff_t *ppos)
{
	char *data = NULL;
	int idx;

	if (datalen >= PAGE_SIZE)
		datalen = PAGE_SIZE - 1;

	/* No partial writes. */
	if (*ppos != 0)
		goto out;

	data = kmalloc(datalen + 1, GFP_KERNEL);
	if (!data)
		goto out;

	*(data + datalen) = '\0';
	
	if (copy_from_user(data, buf, datalen))
		goto out;

	idx = data[0] - '0';
	//printk("data=%s, index=%d, fp=%p\n", data, idx, (void *)f.int_funcs[idx]);
	f.int_funcs[idx](idx);

out:
	kfree(data);
	return datalen;
}

ssize_t	hello_read(struct file *filep, char __user *buf, size_t size, loff_t *offset)
{	
        printk("Usage: echo <option> > /proc/hello-cfi\n");
        printk("Option values:\n");
        printk("\t0\tCall correct function\n");
        printk("\t1\tCall the wrong function but with the same signature\n");
        printk("\t2\tCall a float function with an int function signature\n");
        printk("\t3\tCall a void function with an int function signature\n");
        printk("\t4\tCall into the middle of a function\n");
        printk("\n");
        printk("\tAll other options are undefined, but should be caught by CFI :)\n");
        printk("\n\n");
        printk("------struct foo f member address------\n");
        printk("\tint_funcs: %p\n", (void*)f.int_funcs);
        printk("\tbad_int_funcs: %p\n", (void*)f.bad_int_funcs);
        printk("\tfloat_funcs: %p\n", (void*)f.float_funcs);
        printk("\tnot_entry_begin: %p\n", (void*)f.not_entry_begin);
        printk("\tnot_entry_middle: %p\n", (void*)f.not_entry_middle);

        printk("\n\n------struct foo f member points to------\n");
        printk("\tint_funcs: %p\n", (void*)f.int_funcs[0]);
        printk("\tbad_int_funcs: %p\n", (void*)f.bad_int_funcs[0]);
        printk("\tfloat_funcs: %p\n", (void*)f.float_funcs[0]);
        printk("\tnot_entry_begin: %p\n", (void*)f.not_entry_begin[0]);
        printk("\tnot_entry_middle: %p\n", (void*)f.not_entry_middle[0]);
        printk("\tnot_entry_middle from int_funcs: %p\n", (void*)f.int_funcs[4]);

	return 0;
}

static const struct file_operations cfp_proc_fops = {
	.read		= hello_read,
	.write		= hello_write,
};

static int __init hello_init(void)
{
	if (proc_create("hello_cfi", 0644,NULL, &cfp_proc_fops) == NULL) {
		printk(KERN_ERR "%s: Error creating proc entry\n", __FUNCTION__);
		goto error_return;
	}
	return 0;

error_return:
	return -1;
}

static void __exit hello_exit(void)
{
	remove_proc_entry("hello", NULL);
	printk(KERN_INFO"Deregistering /proc/hello Interface\n");
}

module_init(hello_init);
module_exit(hello_exit);
