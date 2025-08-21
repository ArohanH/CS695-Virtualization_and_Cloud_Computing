#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/sched/signal.h>
#include <linux/sched.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/string.h>

MODULE_DESCRIPTION("Information about child processes of a given process");
MODULE_AUTHOR("Arohan Hazarika");
MODULE_LICENSE("GPL");

static int pid = 1;
module_param(pid,int,0); //ProcessId of the given process

static int __init lkm2_init(void)
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
            printk(KERN_INFO "[LKM2] There doesn't exist any process with PID %d\n",pid);
            return 0;
        }
        printk(KERN_INFO "[LKM2] The given process has a PID %d and its name is %s\n",pid,process->comm);
        struct task_struct *child;
        list_for_each_entry(child, &process->children, sibling) {
            switch (child->__state) {
                case TASK_NOLOAD:  printk(KERN_INFO "[LKM2] Child PID: %d, Name: %s, State: %s\n", child->pid, child->comm,"TASK_NOLOAD"); break;
                case TASK_NEW:  printk(KERN_INFO "[LKM2] Child PID: %d, Name: %s, State: %s\n", child->pid, child->comm,"TASK_NEW"); break;
                case TASK_TRACED:  printk(KERN_INFO "[LKM2] Child PID: %d, Name: %s, State: %s\n", child->pid, child->comm,"TASK_TRACED"); break;
                case TASK_IDLE:  printk(KERN_INFO "[LKM2] Child PID: %d, Name: %s, State: %s\n", child->pid, child->comm,"TASK_IDLE"); break;
                case TASK_NORMAL:  printk(KERN_INFO "[LKM2] Child PID: %d, Name: %s, State: %s\n", child->pid, child->comm,"TASK_NORMAL"); break;
                case TASK_REPORT:  printk(KERN_INFO "[LKM2] Child PID: %d, Name: %s, State: %s\n", child->pid, child->comm,"TASK_REPORT"); break;
                case TASK_RTLOCK_WAIT:  printk(KERN_INFO "[LKM2] Child PID: %d, Name: %s, State: %s\n", child->pid, child->comm,"TASK_RTLOCK_WAIT"); break;
                case TASK_FREEZABLE:  printk(KERN_INFO "[LKM2] Child PID: %d, Name: %s, State: %s\n", child->pid, child->comm,"TASK_FREEZABLE"); break;
                case TASK_FROZEN:  printk(KERN_INFO "[LKM2] Child PID: %d, Name: %s, State: %s\n", child->pid, child->comm,"TASK_FROZEN"); break;
                case TASK_STATE_MAX:  printk(KERN_INFO "[LKM2] Child PID: %d, Name: %s, State: %s\n", child->pid, child->comm,"TASK_STATE_MAX"); break;
                case TASK_RUNNING:  printk(KERN_INFO "[LKM2] Child PID: %d, Name: %s, State: %s\n", child->pid, child->comm,"TASK_RUNNING"); break;
                case TASK_INTERRUPTIBLE:  printk(KERN_INFO "[LKM2] Child PID: %d, Name: %s, State: %s\n", child->pid, child->comm,"TASK_INTERRUPTIBLE"); break;
                case TASK_UNINTERRUPTIBLE:  printk(KERN_INFO "[LKM2] Child PID: %d, Name: %s, State: %s\n", child->pid, child->comm,"TASK_UNINTERRUPTIBLE"); break;
                case __TASK_STOPPED:  printk(KERN_INFO "[LKM2] Child PID: %d, Name: %s, State: %s\n", child->pid, child->comm,"__TASK_STOPPED"); break;
                case TASK_PARKED:  printk(KERN_INFO "[LKM2] Child PID: %d, Name: %s, State: %s\n", child->pid, child->comm,"TASK_PARKED"); break;
                case TASK_DEAD:  printk(KERN_INFO "[LKM2] Child PID: %d, Name: %s, State: %s\n", child->pid, child->comm,"TASK_DEAD"); break;
                case TASK_WAKEKILL:  printk(KERN_INFO "[LKM2] Child PID: %d, Name: %s, State: %s\n", child->pid, child->comm,"TASK_WAKEKILL"); break;
                case TASK_WAKING:  printk(KERN_INFO "[LKM2] Child PID: %d, Name: %s, State: %s\n", child->pid, child->comm,"TASK_WAKING"); break;
                case TASK_ANY:  printk(KERN_INFO "[LKM2] Child PID: %d, Name: %s, State: %s\n", child->pid, child->comm,"TASK_ANY"); break;
                case TASK_KILLABLE:  printk(KERN_INFO "[LKM2] Child PID: %d, Name: %s, State: %s\n", child->pid, child->comm,"TASK_KILLABLE"); break;
                case TASK_STOPPED:  printk(KERN_INFO "[LKM2] Child PID: %d, Name: %s, State: %s\n", child->pid, child->comm,"TASK_STOPPED"); break;
                default:  printk(KERN_INFO "[LKM2] Child PID: %d, Name: %s, State: %s\n", child->pid, child->comm,"UNKNOWN"); break;
            }
        }
        return 0;
}


static void __exit lkm2_exit(void)
{
        printk(KERN_INFO "[LKM2] Module unloaded\n");
}

module_init(lkm2_init);
module_exit(lkm2_exit);