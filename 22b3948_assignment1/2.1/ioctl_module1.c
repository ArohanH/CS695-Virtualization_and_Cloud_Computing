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

//Structure to store data for ioctl calls
struct ioctl_data {
    unsigned long vaddr;
    unsigned long paddr;
    char value_to_be_written;
};

//List of the above structures to store such data
struct ioctl_list_of_data {
    struct ioctl_data* ioctl_data_list;
    unsigned long count;
};

#define DEVICE_NAME "my_ioctl_device"
#define IOCTL_SET_PHYSICAL_ADDRESS _IOW('a', 1, struct ioctl_list_of_data) // Set value
#define IOCTL_GET_PHYSICAL_ADDRESS _IOR('a', 2, struct ioctl_data) // Get value


//Function for page table walk
static unsigned long pgwalk(unsigned long vaddr1){
    struct mm_struct *mm = current->mm;
    pgd_t *pgd;
    p4d_t *p4d;
    pud_t *pud;
    pmd_t *pmd;
    pte_t *pte;
    unsigned long paddr;

    pgd = pgd_offset(mm, vaddr1);
    if (pgd_none(*pgd) || pgd_bad(*pgd)) {
        printk(KERN_INFO "Invalid PGD\n");
        return -1;
    }

    p4d = p4d_offset(pgd, vaddr1);
    if (p4d_none(*p4d) || p4d_bad(*p4d)) {
        printk(KERN_INFO "Invalid P4D\n");
        return -1;
    }

    pud = pud_offset(p4d, vaddr1);
    if (pud_none(*pud) || pud_bad(*pud)) {
        printk(KERN_INFO "Invalid PUD\n");
        return -1;
    }

    pmd = pmd_offset(pud, vaddr1);
    if (pmd_none(*pmd) || pmd_bad(*pmd)) {
        printk(KERN_INFO "Invalid PMD\n");
        return -1;
    }

    if (!(pmd_present(*pmd))) {
        printk(KERN_INFO "Page not present\n");
        return -1;
    }

    pte = pte_offset_kernel(pmd, vaddr1);
    if (pte_none(*pte)) {
        printk(KERN_INFO "Invalid PTE\n");
        return -1;
    }
    paddr = (pte_pfn(*pte) << PAGE_SHIFT) | (vaddr1 & ~PAGE_MASK);
    printk(KERN_INFO "PID: %d Virtual address: %lx Physical address: %lx\n",current->pid,vaddr1,paddr);
    return paddr;
}


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

static int my_ioctl_close(struct inode *inode, struct file *file) {
    printk(KERN_INFO "Device closed\n");
    return 0;
}


static long my_ioctl(struct file *file, unsigned int cmd, unsigned long arg) {
    switch (cmd) {
    case IOCTL_SET_PHYSICAL_ADDRESS:
        struct ioctl_list_of_data user_data_list;
        if (copy_from_user(&user_data_list, (void *)arg, sizeof(struct ioctl_list_of_data))) {
            return -EFAULT;
        }
        unsigned long* vaddr;
        for(unsigned long i=0;i<user_data_list.count;i++){
            vaddr= (unsigned long *) __va(user_data_list.ioctl_data_list[i].paddr);
            //printk(KERN_INFO "%lx:vadd1 %lx:vadd2", vaddr, user_data_list.ioctl_data_list[i].vaddr);
            memmove(vaddr, &user_data_list.ioctl_data_list[i].value_to_be_written, sizeof(*vaddr));
        }
        break;

    case IOCTL_GET_PHYSICAL_ADDRESS:
        // Copy data from kernel space to user space
        struct ioctl_data user_data;
        struct ioctl_data *user_data_input = (struct ioctl_data*) arg;
        user_data.vaddr=user_data_input->vaddr;
        user_data.value_to_be_written=user_data_input->value_to_be_written;
        user_data.paddr=pgwalk(user_data.vaddr);
        if (copy_to_user((void *)arg, &user_data, sizeof(struct ioctl_data))) {
            return -EFAULT;
        }
        break;

    default:
        printk(KERN_WARNING "Invalid ioctl command\n");
        return -EINVAL;
    }

    return 0;
}

// Module initialization
static int __init my_ioctl_init(void) {
    dev_t dev;

    // Allocate a device number dynamically
    if (alloc_chrdev_region(&dev, 0, 1, DEVICE_NAME) < 0) {
        printk(KERN_ERR "Failed to allocate device number\n");
        return -1;
    }
    dev_major = MAJOR(dev);

    // Initialize character device
    cdev_init(&my_cdev, &fops);
    if (cdev_add(&my_cdev, dev, 1) < 0) {
        unregister_chrdev_region(dev, 1);
        printk(KERN_ERR "Failed to add cdev\n");
        return -1;
    }

    // Create a device class
    dev_class = class_create(THIS_MODULE, "my_ioctl_class");
    if (IS_ERR(dev_class)) {
        cdev_del(&my_cdev);
        unregister_chrdev_region(dev, 1);
        printk(KERN_ERR "Failed to create device class\n");
        return PTR_ERR(dev_class);
    }

    // Create a device node
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

// Module cleanup
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
MODULE_DESCRIPTION("A simple ioctl kernel module for VA to PA translation and writing values directly to PA");
