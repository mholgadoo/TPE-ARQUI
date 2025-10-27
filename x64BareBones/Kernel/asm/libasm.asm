GLOBAL cpuVendor
GLOBAL inb
GLOBAL outb
GLOBAL get_registers
GLOBAL rdtsc
GLOBAL cpuid_info
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

; uint64_t rdtsc(void)
; Lee el Time Stamp Counter (TSC) y retorna un valor de 64 bits
rdtsc:
    push rdx
    rdtsc           ; Lee TSC: EDX:EAX
    shl rdx, 32     ; Shift EDX a la parte alta
    or rax, rdx     ; Combina en RAX
    pop rdx
    ret

; void cpuid_info(uint32_t leaf, uint32_t *eax, uint32_t *ebx, uint32_t *ecx, uint32_t *edx)
; Ejecuta CPUID y retorna los valores en los punteros
cpuid_info:
    push rbx
    push r12
    push r13
    push r14
    push r15
    
    mov r12, rsi    ; save eax pointer
    mov r13, rdx    ; save ebx pointer
    mov r14, rcx    ; save ecx pointer
    mov r15, r8     ; save edx pointer
    
    mov eax, edi    ; leaf en eax
    xor ecx, ecx    ; subleaf = 0
    cpuid
    
    mov [r12], eax  ; store eax
    mov [r13], ebx  ; store ebx
    mov [r14], ecx  ; store ecx
    mov [r15], edx  ; store edx
    
    pop r15
    pop r14
    pop r13
    pop r12
    pop rbx
    ret
