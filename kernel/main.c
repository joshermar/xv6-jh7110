#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "defs.h"
#include "sbi.h"

__attribute__((aligned(16))) char stack[4096 * NCPU];
void _hart_entry(void);

void
main()
{
  if (cpuid() == 1) {
    consoleinit();
    printkinit();
    printk("\n");
    printk("hart 1 starting\n");
    printk("booting xv6 kernel\n");

    kinit();            // physical page allocator
    kvminit();          // create kernel page table
    kvminithart();      // turn on paging
    procinit();         // process table
    trapinit();         // trap vectors
    trapinithart();     // install kernel trap vector
    plicinit();         // set up interrupt controller
    plicinithart();     // ask PLIC for device interrupts
    binit();            // buffer cache
    iinit();            // inode table
    fileinit();         // file table
    userinit();         // first user process

    // start other harts
    for (int c=2; c<=NCPU; c++) {
      struct sbiret ret = sbi_hart_start(c, (uint64)_hart_entry, 0);
      if (ret.error) {
        printk("sbi_hart_start error: %ld\n", ret.error);
        panic("main");
      }
    }

  } else {

    printk("hart %d starting\n", cpuid());
    kvminithart();  // turn on paging
    trapinithart(); // install kernel trap vector
    plicinithart(); // ask PLIC for device interrupts
  }

  scheduler();
}
