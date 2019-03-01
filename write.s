# 1 "write.S"
# 1 "<built-in>"
# 1 "<command-line>"
# 31 "<command-line>"
# 1 "/usr/include/stdc-predef.h" 1 3 4
# 32 "<command-line>" 2
# 1 "write.S"
# 1 "include/asm.h" 1
# 2 "write.S" 2





.globl write; .type write, @function; .align 0; write:
 push %ebp;
 movl %esp, %ebp;
 movl 8(%ebp), %ebx
 movl 12(%ebp), %ecx
 movl 16(%ebp), %edx
 movl $4, %eax
 int $0x80;
  cmpl $0, %EAX
 jl err;
err: movl %eax, errno;
fi:
 pop %ebp;
 ret;
