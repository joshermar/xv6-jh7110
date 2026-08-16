.section .text
.global _entry
_entry:
        # U-Boot incorrectly gives 0 for HART ID 1
        addi tp, a0, 1
        la sp, stack0
        li a0, 1024*4
        mul a0, a0, tp
        add sp, sp, a0
        call main
spin:
        j spin
