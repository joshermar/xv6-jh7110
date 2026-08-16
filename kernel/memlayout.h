// Physical memory layout

//
// JH7110 Memory layout
//
// 02000000 -- CLINT
// 0C000000 -- PLIC
// 10000000 -- uart0
// 40000000 -- Start of physical RAM
// 40200000 -- U-Boot will jump to the OS here
// C0000000 -- End of physical RAM (on 2GB model)

// the kernel uses physical memory thus:
// KERNBASE -- entry.S, then kernel text and data
// end -- start of kernel page allocation area
// PHYSTOP -- end RAM used by the kernel

// Core-Local Interrupt Controller (CLINT)
#define CLINT_BASE  0x02000000L
#define CLINT(hart) (CLINT_BASE + (hart) * 4)

// Platform-Level Interrupt Controller (PLIC) here.
#define PLIC                 0x0c000000L
#define PLIC_PRIORITY        (PLIC + 0x0)
#define PLIC_PENDING         (PLIC + 0x1000)
#define PLIC_SENABLE(hart)   (PLIC + 0x2000 + (hart) * 0x100)
#define PLIC_SPRIORITY(hart) (PLIC + 0x200000 + (hart) * 0x2000)
#define PLIC_SCLAIM(hart)    (PLIC + 0x200004 + (hart) * 0x2000)

#define UART0     0x10000000L
#define UART0_IRQ 0x20

// the kernel expects there to be RAM
// for use by the kernel and user pages
// from physical address 0x80000000 to PHYSTOP.
#define KERNBASE 0x40200000L
#define PHYSTOP  (KERNBASE + 128 * 1024 * 1024)

// map the trampoline page to the highest address,
// in both user and kernel space.
#define TRAMPOLINE (MAXVA - PGSIZE)

// map kernel stacks beneath the trampoline,
// each surrounded by invalid guard pages.
#define KSTACK(p) (TRAMPOLINE - ((p) + 1) * 2 * PGSIZE)

// User memory layout.
// Address zero first:
//   text
//   original data and bss
//   fixed-size stack
//   expandable heap
//   ...
//   TRAPFRAME (p->trapframe, used by the trampoline)
//   TRAMPOLINE (the same page as in the kernel)
#define TRAPFRAME (TRAMPOLINE - PGSIZE)

// Our ramdisk can be located anywhere in memory,
// as long as it's out of the way of the kernel.
#define RAMDISK 0x60000000L
