#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "fs.h"
#include "buf.h"

void
ramdisk_rw(struct buf *b, int write) {
  if (b->blockno > FSSIZE)
    panic("ramdisk_rw: exceeded disk boundary");

  void *disk_ptr = (void *)(RAMDISK + b->blockno * BSIZE);

  if (write)
    memmove(disk_ptr, b->data, BSIZE);
  else
    memmove(b->data, disk_ptr, BSIZE);
}
