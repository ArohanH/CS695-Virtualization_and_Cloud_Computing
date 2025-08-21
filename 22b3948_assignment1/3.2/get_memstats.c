#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sysfs.h>
#include <linux/kobject.h>
#include <linux/mm.h>
#include <linux/sched.h>
#include <linux/fs.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Arohan Hazarika");
MODULE_DESCRIPTION("Kernel Module to expose memory stats for a given PID");
MODULE_VERSION("1.0");

static struct kobject *memstats_kobj;  // Kobject for sysfs directory
static int pid_value = -1;              // Default PID value is -1 (no PID)
static int unit_value = 0;              // Default unit: 0 -> "B" (Bytes)

static unsigned long convert_to_bytes(unsigned long value)
{
    switch (unit_value) {
    case 1:  // "K" for Kilobytes
        return value / 1024;
    case 2:  // "M" for Megabytes
        return value / (1024 * 1024);
    default:  // Default "B" for Bytes
        return value;
    }
}

static ssize_t pid_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    return sprintf(buf, "%d\n", pid_value);
}

static ssize_t pid_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
    sscanf(buf, "%d", &pid_value);
    struct task_struct *task;
    struct task_struct *process;
    for_each_process(task){
            if(task->pid == pid_value){
                process=task;
            }
    }
    if(process==NULL){
        pid_value=-1;
        return -EINVAL; 
    }
    else{
        return count;
    }
    return count;
}

static ssize_t virtmem_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    struct task_struct *task;
    unsigned long virtmem;

    if (pid_value != -1) {
        task = get_pid_task(find_vpid(pid_value), PIDTYPE_PID);
        if (task) {
            virtmem = task->mm->total_vm << PAGE_SHIFT;  // Total virtual memory (in bytes)
            put_task_struct(task);
        }
    }
    else{
        return sprintf(buf, "%d\n", -1);
    }
    return sprintf(buf, "%lu\n", convert_to_bytes(virtmem));
}

static ssize_t physmem_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    struct task_struct *task;
    unsigned long physmem;

    if (pid_value != -1) {
        task = get_pid_task(find_vpid(pid_value), PIDTYPE_PID);
        if (task) {
            physmem = get_mm_rss(task->mm) * PAGE_SIZE;  // Resident Set Size (in bytes)
            put_task_struct(task);
        }
    }
    else{
        return sprintf(buf, "%d\n", -1);
    }
    return sprintf(buf, "%lu\n", convert_to_bytes(physmem));
}

static ssize_t unit_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    char *unit_str;

    switch (unit_value) {
    case 1:
        unit_str = "K";
        break;
    case 2:
        unit_str = "M";
        break;
    default:
        unit_str = "B";
        break;
    }
    printk(KERN_INFO "Value of unit_str = %s\n",unit_str);
    return sprintf(buf, "%s\n", unit_str);
}

static ssize_t unit_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
    if (strncmp(buf, "B", 1) == 0) {
        unit_value = 0;  // "B" for Bytes
    } else if (strncmp(buf, "K", 1) == 0) {
        unit_value = 1;  // "K" for Kilobytes
    } else if (strncmp(buf, "M", 1) == 0) {
        unit_value = 2;  // "M" for Megabytes
    } else {
        pr_err("Invalid unit. Allowed values are B, K, M.\n");
        return -EINVAL;  
    }
    return count;
}

static struct kobj_attribute pid_attribute = __ATTR(pid, 0774, pid_show, pid_store);
static struct kobj_attribute virtmem_attribute = __ATTR(virtmem, 0444, virtmem_show, NULL);
static struct kobj_attribute physmem_attribute = __ATTR(physmem, 0444, physmem_show, NULL);
static struct kobj_attribute unit_attribute = __ATTR(unit, 0774, unit_show, unit_store);

static int __init sysfs_memstats_init(void)
{
    int retval;

    memstats_kobj = kobject_create_and_add("mem_stats", kernel_kobj);
    if (!memstats_kobj) {
        pr_err("Failed to create /sys/kernel/mem_stats\n");
        return -ENOMEM;
    }

    retval = sysfs_create_file(memstats_kobj, &pid_attribute.attr);
    if (retval) {
        pr_err("Failed to create pid file\n");
        goto cleanup;
    }

    retval = sysfs_create_file(memstats_kobj, &virtmem_attribute.attr);
    if (retval) {
        pr_err("Failed to create virtmem file\n");
        goto cleanup;
    }

    retval = sysfs_create_file(memstats_kobj, &physmem_attribute.attr);
    if (retval) {
        pr_err("Failed to create physmem file\n");
        goto cleanup;
    }

    retval = sysfs_create_file(memstats_kobj, &unit_attribute.attr);
    if (retval) {
        pr_err("Failed to create unit file\n");
        goto cleanup;
    }

    printk(KERN_INFO "/sys/kernel/mem_stats created with files: pid, virtmem, physmem, and unit\n");
    return 0;

cleanup:
    kobject_put(memstats_kobj);
    return retval;
}

static void __exit sysfs_memstats_exit(void)
{
    sysfs_remove_file(memstats_kobj, &pid_attribute.attr);
    sysfs_remove_file(memstats_kobj, &virtmem_attribute.attr);
    sysfs_remove_file(memstats_kobj, &physmem_attribute.attr);
    sysfs_remove_file(memstats_kobj, &unit_attribute.attr);
    kobject_put(memstats_kobj);

    printk(KERN_INFO "/sys/kernel/mem_stats removed\n");
}

module_init(sysfs_memstats_init);
module_exit(sysfs_memstats_exit);
