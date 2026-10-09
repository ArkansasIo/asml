; ============================================================================
;  ASMLEX - examples/hello.asm
;  Minimal NASM program: exit with a constant status code (42).
; ============================================================================

section .text
    global start

start:
    mov     eax, 42         ; exit status
    xor     edi, edi        ; clear flags
    mov     edi, eax
    mov     eax, 60         ; SYS_exit
    syscall
