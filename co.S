.global co_get_ctx
co_get_ctx:
    movq %rsi, (%rdi)
    movq %rdi, 8(%rdi)
    movq %rbp, 16(%rdi)
    movq $1, 32(%rdi) // rax
    movq %rbx, 40(%rdi)
    movq %rcx, 48(%rdi)
    movq %rdx, 56(%rdi)
    movq %r8,  64(%rdi)
    movq %r9,  72(%rdi)
    movq %r10, 80(%rdi)
    movq %r11, 88(%rdi)
    movq %r12, 96(%rdi)
    movq %r13, 104(%rdi)
    movq %r14, 112(%rdi)
    movq %r15, 120(%rdi)

    // rip
    movq (%rsp), %rcx
    movq %rcx, 128(%rdi)

    // rsp
    leaq 8(%rsp), %rcx
    movq %rcx, 24(%rdi)

    // restore rcx
    movq 48(%rdi), %rcx
    movq $0, %rax

    ret

.global co_set_ctx
co_set_ctx:
    movq (%rdi), %rsi
    movq 16(%rdi), %rbp
    movq 32(%rdi), %rax
    movq 40(%rdi), %rbx
    movq 48(%rdi), %rcx
    movq 56(%rdi), %rdx
    movq 64(%rdi), %r8
    movq 72(%rdi), %r9
    movq 80(%rdi), %r10
    movq 88(%rdi), %r11
    movq 96(%rdi), %r12
    movq 104(%rdi),%r13
    movq 112(%rdi),%r14
    movq 120(%rdi),%r15
    movq 24(%rdi), %rsp
    pushq 128(%rdi)         // rip
    movq 8(%rdi), %rdi      // rdi should be recovered last

    ret
