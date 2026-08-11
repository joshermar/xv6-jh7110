.section .text
.global _entry
_entry:
        # Save HART ID (from bootloader) to TP
        mv tp, a0

        # set up a stack for C.
        # stack0 is declared in start.c,
        # with a 4096-byte stack per CPU.
        # sp = stack0 + ((hartid + 1) * 4096)
        la sp, stack0
        li a0, 1024*4
        addi a1, tp, 1
        mul a0, a0, a1
        add sp, sp, a0

        # jump to main() (we should already be in in S mode)
        call main

spin:
        j spin
