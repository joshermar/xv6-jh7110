#include "types.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "fs.h"
#include "file.h"
#include "riscv.h"
#include "defs.h"

#define CLOCKSPEED 4000000  // JH7110 timebase frequency
#define BUFSZ 20

uint64 realtime = 0; // wall-clock Unix time in microseconds
static uint64 last_cycle = 0; // last CPU time sample
static struct spinlock timelock;

static int
int_to_str(uint64 n, char *buf)
{
  char rev_buf[BUFSZ];
  int len=0;
  do {
    rev_buf[len++] = '0' + n % 10;
    n /= 10;

  } while(n);

  for (int i=len-1; i >= 0; i--)
    *buf++ = rev_buf[i];

  return len;
}

static int
str_to_int(const char *s, uint64 *n)
{
  if ('0' > *s || *s > '9')
    return 0;

  *n = 0;
  while ('0' <= *s && *s <= '9')
    *n = *n * 10 + *s++ - '0';

  return 1;
}

static void
timeupdate()
{
  uint64 this_cycle = r_time();
  realtime += (this_cycle - last_cycle) / (CLOCKSPEED/1000000);
  last_cycle = this_cycle;
}

int
timeread(int user_dst, uint64 dst, int n)
{
  static int pos = 0;
  static int len = 1;
  static char buf[BUFSZ];

  if (pos >= len) {
    // end of readable data
    pos = 0;
    return 0;
  }

  acquire(&timelock);

  if (pos == 0) {
    // start of read
    timeupdate();
    len = int_to_str(realtime/1000000, buf);
  }

  int realn = (n < len) ? n - pos : len - pos;
  int err = either_copyout(user_dst, dst, &buf[pos], realn);
  pos += realn;

  release(&timelock);

  return err ? err : realn;
}

int
timewrite(int user_src, uint64 src, int n)
{
  char buf[BUFSZ] = {};
  uint64 secs;

  if(n > BUFSZ)
    return -1;

  // Time must be written in one go as we
  // don't keep track of the file position.

  acquire(&timelock);

  int err = either_copyin(buf, user_src, src, n);
  if (!err && str_to_int(buf, &secs)) {
    realtime = secs*1000000;
    last_cycle = r_time();
  }

  timeupdate();

  release(&timelock);

  return err ? err : n;
}

void
timeinit()
{
  initlock(&timelock, "timelock");
  devsw[TIME].read = timeread;
  devsw[TIME].write = timewrite;
}
