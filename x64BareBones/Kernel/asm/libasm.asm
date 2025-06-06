GLOBAL cpuVendor
GLOBAL inb
GLOBAL outb
GLOBAL save_registers
section .data

section .text

; void save_registers(uint64_t *dest);
save_registers:
    mov [rdi + 0*8], rax
    mov [rdi + 1*8], rbx
    mov [rdi + 2*8], rcx
    mov [rdi + 3*8], rdx
    mov [rdi + 4*8], rsi
    mov [rdi + 5*8], rdi
    mov [rdi + 6*8], rbp
    mov [rdi + 7*8], rsp
    mov [rdi + 8*8], r8
    mov [rdi + 9*8], r9
    mov [rdi + 10*8], r10
    mov [rdi + 11*8], r11
    mov [rdi + 12*8], r12
    mov [rdi + 13*8], r13
    mov [rdi + 14*8], r14
    mov [rdi + 15*8], r15
    ret


cpuVendor:
	push rbp
	mov rbp, rsp

	push rbx

	mov rax, 0
	cpuid


	mov [rdi], ebx
	mov [rdi + 4], edx
	mov [rdi + 8], ecx

	mov byte [rdi+13], 0

	mov rax, rdi

	pop rbx

	mov rsp, rbp
	pop rbp
	ret

outb:
    mov dx, di
    mov al, sil
    out dx, al
    ret

inb:
    mov dx, di
    in al, dx
    movzx rax, al
    ret
