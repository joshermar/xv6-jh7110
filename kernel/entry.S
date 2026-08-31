.section .text
.global _entry
.global _hart_entry
_entry:
        # Add 1 as U-boot mistakenly numbers the boot core 0
        addi a0, a0, 1
_hart_entry:
        # Subsecuent cores are explicitly invoked from an
        # SBI call, hence they are numbered correctly
        mv tp, a0
        la sp, stack
        li a0, 1024*4
        mul a0, a0, tp
        add sp, sp, a0
        call main
spin:
        wfi
        j spin
