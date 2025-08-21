#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/mm.h>
#include <linux/page-flags.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Arohan Hazarika");
MODULE_DESCRIPTION("Page Faults Kernel Module");
MODULE_VERSION("1.0");

#define PROC_NAME "get_pgfaults"

static int pg_faults_show(struct seq_file *m, void *v)
{
    unsigned long list_of_events[NR_VM_EVENT_ITEMS];
    all_vm_events(list_of_events);
    unsigned long faults = list_of_events[PGFAULT];
    seq_printf(m, "Page Faults: %lu\n", faults);  
    return 0;
}

static int pg_faults_open(struct inode *inode, struct file *file)
{
    return single_open(file, pg_faults_show, NULL);
}

static const struct proc_ops get_pgfaults_fops = {
    .proc_open = pg_faults_open,   
    .proc_read = seq_read,         
    .proc_release = single_release 
};

static int __init pgfaults_procfs_init(void)
{
    struct proc_dir_entry *entry;

    entry = proc_create(PROC_NAME, 0666, NULL, &get_pgfaults_fops);
    if (!entry) {
        pr_err("Unable to create /proc/%s\n", PROC_NAME);
        return -ENOMEM;  
    }

    printk(KERN_INFO "/proc/%s created successfully!\n", PROC_NAME);
    return 0;
}

static void __exit pgfaults_procfs_exit(void)
{
    remove_proc_entry(PROC_NAME, NULL);
    printk(KERN_INFO "/proc/%s removed\n", PROC_NAME);
}

module_init(pgfaults_procfs_init);
module_exit(pgfaults_procfs_exit);
