#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/sched/signal.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/mm.h>
#include <linux/pagewalk.h>
#include <linux/mm_types.h>

MODULE_DESCRIPTION("Calculate Virtual and Physical address space of a process");
MODULE_AUTHOR("Arohan Hazarika");
MODULE_LICENSE("GPL");

static int pid = 1;
module_param(pid,int,0); //ProcessId of the given process

// static long vaddr = 0;
// module_param(vaddr,long,0);

static void print_vas_pas(struct task_struct *p){
    struct mm_struct *mm = p->mm;
    printk(KERN_INFO "[LKM4] PID: %d Virtual address Space: %lu Physical address Space: %lu\n",pid,mm->total_vm*PAGE_SIZE/1024,get_mm_rss(mm)*PAGE_SIZE/1024);
}

static int __init lkm4_init(void)
{
        //printk(KERN_NOTICE "Hello World\n");
        struct task_struct *p;
        struct task_struct *process=NULL;
        for_each_process(p){
            if(p->pid == pid){
                process=p;
            }
        }
        if(process==NULL){
            printk(KERN_INFO "There doesn't exist any process with PID %d\n",pid);
            return 0;
        }
        printk(KERN_INFO "The given process has a PID %d and its name is %s\n",pid,process->comm);
        print_vas_pas(process);
        return 0;
}


static void __exit lkm4_exit(void)
{
        printk(KERN_INFO "Module unloaded\n");
}

module_init(lkm4_init);
module_exit(lkm4_exit);