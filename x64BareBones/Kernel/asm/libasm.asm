GLOBAL cpuVendor
GLOBAL inb
GLOBAL outb
GLOBAL get_registers
EXTERN snapshot

section .data

section .text

get_registers:
    lea rax, [rel snapshot]
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
