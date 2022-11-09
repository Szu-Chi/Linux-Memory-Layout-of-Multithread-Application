#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/pid.h>
#include <linux/sched.h>
#include <linux/syscalls.h>
#include <linux/mm.h>
#include <linux/highmem.h>
#include <asm/pgtable.h>
#include <asm/pgalloc.h>
#include <asm/io.h>

SYSCALL_DEFINE2(get_phy_addr, int, pid, unsigned long, vaddr){
    struct task_struct *task = NULL;
    struct vm_area_struct *vm_temp = NULL;
    struct mm_struct *mm = NULL;

    task = pid_task(find_vpid(pid), PIDTYPE_PID);
    if(task == NULL) {
        printk("%s: get pid task struct failed\n", __func__);
        return -1;
    }

    mm = task->mm;


    pgd_t *pgd;
    p4d_t *p4d;
    pud_t *pud;
    pmd_t *pmd;
    pte_t *ptep,pte;

    struct page *page = NULL;
    unsigned long physical_address;

    pgd = pgd_offset(mm, vaddr);
    if (pgd_none(*pgd) || pgd_bad(*pgd))
	goto out;
    printk(KERN_NOTICE "Valid pgd");

    p4d = p4d_offset(pgd, vaddr);
    if (p4d_none(*p4d) || p4d_bad(*p4d))
    	goto out;
    printk(KERN_NOTICE "Valid p4d");


    pud = pud_offset(p4d, vaddr);
    if (pud_none(*pud) || pud_bad(*pud))
    	goto out;
    printk(KERN_NOTICE "Valid pud");

    pmd = pmd_offset(pud, vaddr);
    if (pmd_none(*pmd) || pmd_bad(*pmd))
	goto out;
    printk(KERN_NOTICE "Valid pmd");

    ptep = pte_offset_map(pmd, vaddr);
    if (!ptep)
	goto out;
    pte = *ptep;

    page = pte_page(pte);
    if (page)
    physical_address = page_to_phys(page);
    pte_unmap(ptep);

    printk(  
             "virtual address = 0x%012lx ----> physical address = 0x%012lx\n",
             (unsigned long)vaddr,  (unsigned long)physical_address
      );
out:
    return physical_address;
}
