    .section .text.entry
    .globl _entry
_entry:
    la sp, boot_stack_top   # 设置好 os 运行的堆栈
    call main

    .section .bss.stack
    .globl boot_stack
boot_stack:
    .space 4096 * 16        # os内核堆栈运行在这上面
    .globl boot_stack_top
boot_stack_top:
