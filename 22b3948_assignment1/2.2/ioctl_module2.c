#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h> // For copy_to_user and copy_from_user
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/errno.h>
#include <linux/ioctl.h>
#include <linux/mm.h>
#include <asm/current.h>
#include <linux/sched.h>
#include <linux/pagewalk.h>
#include <linux/mm_types.h>
#include <linux/delay.h>

#define DEVICE_NAME "my_ioctl_device_2"
#define IOCTL_CHANGE_PARENT_PID _IOWR('a', 1, int) 
#define IOCTL_KILL_CHILDREN _IOWR('a', 2, int) 

static int dev_major;        
static struct class *dev_class;
static struct cdev my_cdev;

static int my_ioctl_open(struct inode *inode, struct file *file);
static int my_ioctl_close(struct inode *inode, struct file *file);
static long my_ioctl(struct file *file, unsigned int cmd, unsigned long arg);

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = my_ioctl_open,
    .release = my_ioctl_close,
    .unlocked_ioctl = my_ioctl,
};

static int my_ioctl_open(struct inode *inode, struct file *file) {
    printk(KERN_INFO "Device opened\n");
    return 0;
}

// Close function
static int my_ioctl_close(struct inode *inode, struct file *file) {
    printk(KERN_INFO "Device closed\n");
    return 0;
}

// Ioctl function
static long my_ioctl(struct file *file, unsigned int cmd, unsigned long arg) {
    switch (cmd) {
    case IOCTL_CHANGE_PARENT_PID:

        struct task_struct *new_parent1;
        struct task_struct *curr = get_current();
        struct task_struct *p;
        int pid1;
        if (copy_from_user(&pid1, (unsigned long __user *)arg, sizeof(int))) {
            return -EFAULT;
        }
        printk(KERN_INFO "Pid %d made a request\n",pid1);

        bool found = false;
        for_each_process(p) {
            if (p->pid == pid1) {
                found = true;
                new_parent1 = p;
            }
        }
        if (!found) {
            return -EFAULT;
        }

        // rcu_read_lock();        
        task_lock(curr); 
        list_del_init(&curr->sibling); 
        curr->parent = new_parent1; 
        curr->real_parent = new_parent1;
        list_add_tail(&curr->sibling, &new_parent1->children);
        task_unlock(curr);
        task_lock(new_parent1);
        new_parent1->signal->group_exit_code = 0;
        new_parent1->flags |= PF_SIGNALED;
        task_unlock(new_parent1);
        // rcu_read_unlock();   
        break;

    case IOCTL_KILL_CHILDREN:
        struct task_struct *new_parent2;
        int pid2;
        struct task_struct* process;

        if (copy_from_user(&pid2, (void *)arg, sizeof(int))) {
            return -EFAULT;
        }

        for_each_process(new_parent2) {
            if (new_parent2->pid == pid2) {
                process = new_parent2;
                break;
            }
        } 
    
        struct task_struct *child;
        rcu_read_lock();
        list_for_each_entry(child, &process->children, sibling) {
            send_sig(SIGKILL,child,0);
        }
        rcu_read_unlock();
        msleep(30);
        send_sig(SIGTERM,process,0);
        break;

    default:
        printk(KERN_WARNING "Invalid ioctl command\n");
        return -EINVAL;
    }

    return 0;
}

static int __init my_ioctl_init(void) {
    dev_t dev;

    if (alloc_chrdev_region(&dev, 0, 1, DEVICE_NAME) < 0) {
        printk(KERN_ERR "Failed to allocate device number\n");
        return -1;
    }
    dev_major = MAJOR(dev);

    cdev_init(&my_cdev, &fops);
    if (cdev_add(&my_cdev, dev, 1) < 0) {
        unregister_chrdev_region(dev, 1);
        printk(KERN_ERR "Failed to add cdev\n");
        return -1;
    }

    dev_class = class_create(THIS_MODULE, "my_ioctl_class");
    if (IS_ERR(dev_class)) {
        cdev_del(&my_cdev);
        unregister_chrdev_region(dev, 1);
        printk(KERN_ERR "Failed to create device class\n");
        return PTR_ERR(dev_class);
    }

    if (device_create(dev_class, NULL, dev, NULL, DEVICE_NAME) == NULL) {
        class_destroy(dev_class);
        cdev_del(&my_cdev);
        unregister_chrdev_region(dev, 1);
        printk(KERN_ERR "Failed to create device node\n");
        return -1;
    }

    printk(KERN_INFO "Kernel module loaded. Device created: /dev/%s\n", DEVICE_NAME);
    return 0;
}

static void __exit my_ioctl_exit(void) {
    dev_t dev = MKDEV(dev_major, 0);

    device_destroy(dev_class, dev);
    class_destroy(dev_class);
    cdev_del(&my_cdev);
    unregister_chrdev_region(dev, 1);
    printk(KERN_INFO "Kernel module unloaded\n");
}

module_init(my_ioctl_init);
module_exit(my_ioctl_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Arohan Hazarika");
MODULE_DESCRIPTION("A simple ioctl kernel module to change parent of a process and to kill a process' children");
