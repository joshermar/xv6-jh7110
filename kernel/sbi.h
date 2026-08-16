#ifndef SBI_H
#define SBI_H

#define SBI_EXT_CONSOLE_PUTCHAR 0x1
#define SBI_EXT_CONSOLE_GETCHAR 0x2
#define SBI_EXT_SHUTDOWN 0x8
#define SBI_EXT_SET_TIMER 0x54494D45

struct sbiret {
  uint64 error;
  uint64 value;
};

static inline struct sbiret
sbi_ecall(uint64 ext, uint64 fid, uint64 arg0, uint64 arg1,
          uint64 arg2, uint64 arg3, uint64 arg4, uint64 arg5)
{
  register uint64 a0 asm("a0") = arg0;
  register uint64 a1 asm("a1") = arg1;
  register uint64 a2 asm("a2") = arg2;
  register uint64 a3 asm("a3") = arg3;
  register uint64 a4 asm("a4") = arg4;
  register uint64 a5 asm("a5") = arg5;
  register uint64 a6 asm("a6") = fid;
  register uint64 a7 asm("a7") = ext;

  asm volatile("ecall"
              : "+r"(a0), "+r"(a1)
              : "r"(a2), "r"(a3), "r"(a4), "r"(a5), "r"(a6), "r"(a7)
              : "memory");

  return (struct sbiret){ a0, a1 };
}

static struct sbiret
sbi_console_putchar(int ch) {
  return sbi_ecall(SBI_EXT_CONSOLE_PUTCHAR, 0, ch, 0, 0, 0, 0, 0);
}

static struct sbiret
sbi_shutdown() {
  return sbi_ecall(SBI_EXT_SHUTDOWN, 0, 0, 0, 0, 0, 0, 0);
}

static struct sbiret
sbi_set_timer(uint64 stime)
{
  return sbi_ecall(SBI_EXT_SET_TIMER, 0, stime, 0, 0, 0, 0, 0);
}

#endif // SBI_H
