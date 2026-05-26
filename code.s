.globl test
test:
    mov %rdi, -8(%rbp)
    mov %rsi, -16(%rbp)
    mov -8(%rbp), %rdi
    mov -8(%rbp), %rsi
    mov -8(%rbp), %rdx