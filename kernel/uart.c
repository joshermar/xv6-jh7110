//
// low-level driver for dw_apb_uart UART
//

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "spinlock.h"
#include "proc.h"
#include "defs.h"

#define READ(reg)     (*((volatile uint32 *)(UART0 + reg)))
#define WRITE(reg, v) (*((volatile uint32 *)(UART0 + reg)) = (v))

// dw_apb_uart is mostly compatible with a standard 16550 UART,
// but with 32 bit wide registers and a few other quirks.
#define RBR         0x00 // Receive Buffer Register
#define THR         0x00 // Transmit Holding Register
#define DLL         0x00 // Divisor Latch Low
#define IER         0x04 // Interrupt Enable Register
#define DLH         0x04 // Divisor Latch High
#define IIR         0x08 // Interrupt Identity Register
#define FCR         0x08 // FIFO Control Register
#define LCR         0x0c // Line Control Register
#define LSR         0x14 // Line Status Register
#define SCR         0x1c // Scratchpad Register
#define USR         0x7c // UART Status Register
#define SRR         0x88 // Software Reset Register

// register fields
#define IER_ERBFI  (1 << 0) // Rx data available interrupt
#define IER_ETBEI  (1 << 1) // Tx data holding register empty (THRE) interrupt
#define FCR_FIFOE  (1 << 0) // FIFO Enable
#define FCR_RFIFOR (1 << 1) // Reset Rx FIFO
#define FCR_XFIFOR (1 << 2) // Reset Tx FIFO
#define IIR_NOINT  (1 << 0) // IID: None (no interrupt pending)
#define IIR_THRE   (1 << 1) // IID: THRE
#define IIR_RDA    (1 << 2) // IID: Received data avaiable
#define LCR_DLS    (3 << 0) // Data Length Select (0b11 = 8 bits)
#define LCR_DLAB   (1 << 7) // Divisor Latch Access Bit
#define LSR_DR     (1 << 0) // Data Ready for read
#define LSR_THRE   (1 << 5) // Transmit Holding Register Empty
#define USR_BUSY   (1 << 0) // UART is busy
#define USR_TFNF   (1 << 1) // Tx FIFO is NOT full
#define SRR_UR     (1 << 0) // UART Reset

extern volatile int panicked;  // from printk.c

static struct spinlock tx_lock;
static int tx_chan; // &tx_chan is the "wait channel"

void
uartinit(void)
{
  // disable interrupts before befor UART is configured
  WRITE(IER, 0x00);

  // UART should NOT be busy when writting LCR bits
  while (READ(USR) & USR_BUSY)
    ;
  // set divisor latch for baudrate of 115200
  WRITE(LCR, LCR_DLAB);
  WRITE(DLL, 0x0d);
  WRITE(DLH, 0x00);

  // set word length to 8 bits, no parity
  WRITE(LCR, LCR_DLS);

  // enable and reset FIFO
  WRITE(FCR, FCR_FIFOE | FCR_RFIFOR | FCR_XFIFOR );

  // enable TX/RX interrupts
  WRITE(IER, IER_ERBFI | IER_ETBEI );
}

void
uartputc(int c)
{
  acquire(&tx_lock);

  while ((READ(USR) & USR_TFNF) == 0) {
    // wait for a UART THRE interrupt
    sleep(&tx_chan, &tx_lock);
  }

  WRITE(THR, c);

  release(&tx_lock);
}

// write a byte to the uart without using
// interrupts, for use by kernel printk() and
// to echo characters. it spins waiting for the uart's
// output register to be empty.
void
uartputc_sync(int c)
{
  push_off();

  // TODO: Halt the cores explicitly during panic
  if (panicked) {
    for (;;)
      asm volatile("wfi");
  }

  while ((READ(USR) & USR_TFNF) == 0)
    ;
  WRITE(THR, c);

  pop_off();
}

static void
tx_wake() {
  acquire(&tx_lock);

  // This should always be the case??
  if (READ(LSR) & LSR_THRE)
    wakeup(&tx_chan);

  release(&tx_lock);
}

// handle a uart interrupt, raised because input has
// arrived, or the uart is ready for more output, or
// both. called from devintr().
void
uartintr(void)
{

  uint8 iid = 0x1;
  while ((iid = READ(IIR) & 0xf) != 0x1) {

    switch (iid) {

    // UART is ready to start transmitting again
    case IIR_THRE:
      tx_wake();
      break;

    // data is available to read
    case IIR_RDA:
      consoleintr(READ(RBR));
      break;

    default:
      printk("uartintr: unhandled iid '%x'", iid);
    }
  }
}
