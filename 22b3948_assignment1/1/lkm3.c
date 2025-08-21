#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/sched/signal.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/mm.h>
#include <linux/pagewalk.h>
#include <linux/mm_types.h>

MODULE_DESCRIPTION("Conversion of VA to PA for a given PID");
MODULE_AUTHOR("Arohan Hazarika");
MODULE_LICENSE("GPL");

static int pid = 1;
module_param(pid,int,0); //ProcessId of the given process

static long vaddr = 0;
module_param(vaddr,long,0);

static void pgwalk(struct task_struct *p){
    struct mm_struct *mm = p->mm;
    pgd_t *pgd;
    p4d_t *p4d;
    pud_t *pud;
    pmd_t *pmd;
    pte_t *pte;
    phys_addr_t paddr;

    pgd = pgd_offset(mm, vaddr);
    if (pgd_none(*pgd) || pgd_bad(*pgd)) {
        printk(KERN_INFO "[LKM3] Invalid PGD\n");
        return;
    }

    p4d = p4d_offset(pgd, vaddr);
    if (p4d_none(*p4d) || p4d_bad(*p4d)) {
        printk(KERN_INFO "[LKM3] Invalid P4D\n");
        return;
    }

    pud = pud_offset(p4d, vaddr);
    if (pud_none(*pud) || pud_bad(*pud)) {
        printk(KERN_INFO "[LKM3] Invalid PUD\n");
        return;
    }

    pmd = pmd_offset(pud, vaddr);
    if (pmd_none(*pmd) || pmd_bad(*pmd)) {
        printk(KERN_INFO "[LKM3] Invalid PMD\n");
        return;
    }

    if (!(pmd_present(*pmd))) {
        printk(KERN_INFO "[LKM3] Page not present\n");
        return;
    }

    pte = pte_offset_kernel(pmd, vaddr);
    if (pte_none(*pte)) {
        printk(KERN_INFO "[LKM3] Invalid PTE\n");
        return;
    }
    paddr = (phys_addr_t)(pte_pfn(*pte) << PAGE_SHIFT) | (vaddr & ~PAGE_MASK);
    printk(KERN_INFO "[LKM3] PID: %d Virtual address: %lx Physical address: %lx\n",pid,vaddr,paddr);
}

static int __init lkm3_init(void)
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
            printk(KERN_INFO "[LKM3] There doesn't exist any process with PID %d\n",pid);
            return 0;
        }
        printk(KERN_INFO "[LKM3] The given process has a PID %d and its name is %s\n",pid,process->comm);
        pgwalk(process);
        return 0;
}


static void __exit lkm3_exit(void)
{
        printk(KERN_INFO "[LKM3] Module unloaded\n");
}

module_init(lkm3_init);
module_exit(lkm3_exit);