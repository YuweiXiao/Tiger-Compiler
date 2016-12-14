.text
.globl tigermain
.type tigermain, @function
 tigermain:
pushl %ebp
movl %esp, %ebp
addl $-36, %esp
L13:
movl %edi, -12(%ebp)
movl %esi, -16(%ebp)
movl %ebx, -20(%ebp)
movl $16, %ebx
movl %ebx, -4(%ebp)
movl %ebp, %ebx
addl $-8, %ebx
movl %ebx, -24(%ebp)
movl -4(%ebp), %ebx
movl %ebx, %edi
movl %edi, %ebx
imull $4, %ebx
pushl %ebx
call tMalloc
movl %eax, %esi
movl $0, %ecx
movl $0, %edx
movl %esi, %ebx
L0:
cmp %edi, %ecx
jl L1
L2:
movl -24(%ebp), %ebx
movl %esi, (%ebx)
pushl %ebp
call try
movl -20(%ebp), %ebx
movl -16(%ebp), %esi
movl -12(%ebp), %edi
jmp L12
L1:
movl %edx, (%ebx)
addl $1, %ecx
addl $4, %ebx
jmp L0
L12:
leave
ret




.text
.globl try
.type try, @function
 try:
pushl %ebp
movl %esp, %ebp
addl $-24, %esp
L15:
movl %edi, %edi
movl %edi, -4(%ebp)
movl %esi, %esi
movl %esi, -8(%ebp)
movl %ebx, %ebx
movl %ebx, -12(%ebp)
movl 8(%ebp), %ebx
pushl %ebx
call init
movl 8(%ebp), %ebx
movl %ebx, %edi
movl 8(%ebp), %edx
movl $0, %esi
movl 8(%ebp), %ebx
movl -4(%ebx), %ecx
movl %ecx, %ecx
subl $1, %ecx
movl $7, %ebx
pushl %ebx
pushl %ecx
pushl %esi
pushl %edx
call bsearch
movl %eax, %eax
pushl %eax
pushl %edi
call printi
movl 8(%ebp), %ebx
pushl .L11
pushl %ebx
call print
movl -12(%ebp), %ebx
movl %ebx, %ebx
movl -8(%ebp), %esi
movl %esi, %esi
movl -4(%ebp), %edi
movl %edi, %edi
jmp L14
L14:
leave
ret




.L11: .string "\n"
.text
.globl bsearch
.type bsearch, @function
 bsearch:
pushl %ebp
movl %esp, %ebp
addl $-24, %esp
L17:
movl %edi, %edi
movl %edi, -8(%ebp)
movl %esi, %esi
movl %esi, -12(%ebp)
movl %ebx, %edi
movl 12(%ebp), %ebx
movl %ebx, %ebx
movl 16(%ebp), %ecx
movl 12(%ebp), %ebx
cmp %ecx, %ebx
je L9
L10:
movl %eax, %ecx
movl %edx, %edx
movl 12(%ebp), %eax
movl 16(%ebp), %ebx
addl %ebx, %eax
movl %eax, %eax
cltd
movl $2, %ebx
idivl %ebx
movl %eax, %ebx
movl %ecx, %eax
movl %edx, %edx
movl %ebx, -4(%ebp)
movl 8(%ebp), %edx
movl -4(%ebp), %esi
movl %esi, %esi
addl $1, %esi
movl 16(%ebp), %ecx
movl 20(%ebp), %ebx
pushl %ebx
pushl %ecx
pushl %esi
pushl %edx
call bsearch
movl %eax, %eax
movl 20(%ebp), %edx
movl 8(%ebp), %ebx
movl -8(%ebx), %ecx
movl -4(%ebp), %ebx
movl %ebx, %ebx
imull $4, %ebx
addl %ebx, %ecx
movl (%ecx), %ebx
cmp %edx, %ebx
jl L7
L8:
movl 8(%ebp), %esi
movl 12(%ebp), %edx
movl -4(%ebp), %ecx
movl 20(%ebp), %ebx
pushl %ebx
pushl %ecx
pushl %edx
pushl %esi
call bsearch
movl %eax, %eax
L7:
movl %eax, %ebx
L9:
movl %edi, %ebx
movl -12(%ebp), %esi
movl %esi, %esi
movl -8(%ebp), %edi
movl %edi, %edi
jmp L16
L16:
leave
ret




.text
.globl init
.type init, @function
 init:
pushl %ebp
movl %esp, %ebp
addl $-20, %esp
L19:
movl %edi, %edi
movl %esi, %esi
movl %ebx, %ebx
movl $0, %ebx
movl %ebx, -4(%ebp)
movl 8(%ebp), %ebx
movl -4(%ebx), %ebx
movl %ebx, %ebx
subl $1, %ebx
movl %ebx, -8(%ebp)
L4:
movl -8(%ebp), %ecx
movl -4(%ebp), %ebx
cmp %ecx, %ebx
jle L5
L6:
movl %ebx, %ebx
movl %esi, %esi
movl %edi, %edi
jmp L18
L5:
movl -4(%ebp), %ebx
movl %ebx, %ebx
imull $2, %ebx
movl %ebx, %edx
addl $1, %edx
movl 8(%ebp), %ebx
movl -8(%ebx), %ecx
movl -4(%ebp), %ebx
movl %ebx, %ebx
imull $4, %ebx
addl %ebx, %ecx
movl %edx, (%ecx)
movl 8(%ebp), %ebx
pushl %ebx
call nop
movl -4(%ebp), %ebx
movl %ebx, %ebx
addl $1, %ebx
movl %ebx, -4(%ebp)
jmp L4
L18:
leave
ret




.text
.globl nop
.type nop, @function
 nop:
pushl %ebp
movl %esp, %ebp
addl $-12, %esp
L21:
movl %edi, %edi
movl %esi, %esi
movl %ebx, %ebx
movl 8(%ebp), %ebx
pushl .L3
pushl %ebx
call print
movl %ebx, %ebx
movl %esi, %esi
movl %edi, %edi
jmp L20
L20:
leave
ret




.L3: .string ""
