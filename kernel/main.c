#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "defs.h"

volatile static int started = 0;

// start() jumps here in supervisor mode on all CPUs.
void
main()
{
  if (cpuid() == 0) {
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

    panic("boop!\n");

    binit();            // buffer cache
    iinit();            // inode table
    fileinit();         // file table
    virtio_disk_init(); // emulated hard disk
    userinit();         // first user process

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
