#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/sched/signal.h>
#include <linux/sched.h>
#include <linux/init.h>
#include <linux/module.h>

MODULE_DESCRIPTION("Information about all processes");
MODULE_AUTHOR("Arohan Hazarika");
MODULE_LICENSE("GPL");

static int lkm1_init(void)
{
        //printk(KERN_NOTICE "Hello World\n");
        struct task_struct *p;
	    int pid;
        printk(KERN_INFO "[LKM1] Runnable Processes:\n");
        printk(KERN_INFO "[LKM1] PID       PROC\n");
        for_each_process(p){
            if(p->__state == TASK_RUNNING){
                 printk(KERN_INFO "[LKM1] %d       %s\n", p->pid,p->comm);
            }
        }
        return 0;
}

static void lkm1_exit(void)
{
        printk(KERN_INFO "[LKM1] Module unloaded\n");
}

module_init(lkm1_init);
module_exit(lkm1_exit);