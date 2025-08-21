#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/sched/signal.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/mm.h>
#include <linux/pagewalk.h>
#include <linux/mm_types.h>
#include <linux/maple_tree.h>
#include <linux/hugetlb.h>

MODULE_DESCRIPTION("Module to calculate the number of Trans Huge Pages");
MODULE_AUTHOR("Arohan Hazarika");
MODULE_LICENSE("GPL");

static int pid = 1;
module_param(pid,int,0); //ProcessId of the given process

// static long vaddr = 0;
// module_param(vaddr,long,0);

static void calculate_thp_pages(struct task_struct *p){
    struct mm_struct *mm = p->mm;
    unsigned long index = 0;
    struct vm_area_struct *vma;
    unsigned long thp_count=0;
    unsigned long addr=0;
    down_read(&mm->mmap_lock);
    mt_for_each(&mm->mm_mt,vma,index,ULONG_MAX){
        //Method1
        for (addr = vma->vm_start; addr < vma->vm_end; addr += PAGE_SIZE) {
            pgd_t *pgd;
            p4d_t *p4d;
            pud_t *pud;
            pmd_t *pmd;

            pgd = pgd_offset(mm, addr);
            

            p4d = p4d_offset(pgd, addr);
             if (!(p4d_present(*p4d))) {
                continue;
            }

            pud = pud_offset(p4d, addr);
            if (!(pud_present(*pud))) {
                continue;
            }

            pmd = pmd_offset(pud, addr);

            if (!(pmd_present(*pmd))) {
                continue;
            }

            if(pmd_trans_huge(*pmd)){
                thp_count++;
                addr += HPAGE_PMD_SIZE - PAGE_SIZE;
            }

        }
    }
    up_read(&mm->mmap_lock);
    printk(KERN_INFO "[LKM5] THP Size: %d KiB, THP count: %d\n",thp_count*HPAGE_PMD_SIZE/1024,thp_count);
}

static int __init lkm5_init(void)
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
            printk(KERN_INFO "[LKM5] There doesn't exist any process with PID %d\n",pid);
            return 0;
        }
        printk(KERN_INFO "[LKM5] The given process has a PID %d and its name is %s\n",pid,process->comm);
        calculate_thp_pages(process);
        return 0;
}


static void __exit lkm5_exit(void)
{
    printk(KERN_INFO "[LKM5] Module unloaded\n");
}

module_init(lkm5_init);
module_exit(lkm5_exit);