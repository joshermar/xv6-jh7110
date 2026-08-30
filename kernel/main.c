#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "defs.h"

// entry.S needs one stack per CPU.
__attribute__((aligned(16))) char stack0[4096 * NCPU];

volatile static int started = 0;

char *magic_word = (char *)0x55000000L;

// start() jumps here in supervisor mode on all CPUs.
void
main()
{
  if (cpuid() == 1) {
    consoleinit();
    printkinit();
    printk("\n");
    printk("xv6 kernel is booting\n");
    printk("printkinit DONE!\n");

    kinit();            // physical page allocator
    printk("kinit DONE!\n");

    kvminit();          // create kernel page table
    printk("kvminit DONE!\n");

    kvminithart();      // turn on paging
    printk("kvminithart DONE!\n");

    procinit();         // process table
    printk("procinit DONE!\n");

    trapinit();         // trap vectors
    printk("trapinit DONE!\n");

    trapinithart();     // install kernel trap vector
    printk("trapinithart DONE!\n");

    plicinit();         // set up interrupt controller
    printk("plicinit DONE!\n");

    plicinithart();     // ask PLIC for device interrupts
    printk("plicinithart DONE!\n");

    binit();            // buffer cache
    printk("binit DONE!\n");

    iinit();            // inode table
    printk("iinit DONE!\n");

    fileinit();         // file table
    printk("fileinit DONE!\n");

    timeinit();
    printk("timeinit DONE!\n");

    userinit();         // first user process
    printk("userinit DONE!\n");


    __atomic_store_n(&started, 1, __ATOMIC_RELEASE);
  } else {
    while (__atomic_load_n(&started, __ATOMIC_ACQUIRE) == 0)
      ;

    printk("hart %d starting\n", cpuid());
    kvminithart();  // turn on paging
    trapinithart(); // install kernel trap vector
    plicinithart(); // ask PLIC for device interrupts
  }

  scheduler();
}
