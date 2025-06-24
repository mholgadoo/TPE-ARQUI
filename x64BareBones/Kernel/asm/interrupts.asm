
GLOBAL _cli
GLOBAL _sti
GLOBAL picMasterMask
GLOBAL picSlaveMask
GLOBAL haltcpu
GLOBAL _hlt

GLOBAL _irq00Handler
GLOBAL _irq01Handler
GLOBAL _irq02Handler
GLOBAL _irq03Handler
GLOBAL _irq04Handler
GLOBAL _irq05Handler

GLOBAL _int80Handler

GLOBAL _exception0Handler
GLOBAL _exception6Handler

EXTERN irqDispatcher
EXTERN exceptionDispatcher
EXTERN syscallDispatcher
EXTERN getStackBase

SECTION .text

%macro pushState 0
	push rax
	push rbx
	push rcx
	push rdx
	push rbp
	push rdi
	push rsi
	push r8
	push r9
	push r10
	push r11
	push r12
	push r13
	push r14
	push r15
%endmacro

%macro popState 0
	pop r15
	pop r14
	pop r13
	pop r12
	pop r11
	pop r10
	pop r9
	pop r8
	pop rsi
	pop rdi
	pop rbp
	pop rdx
	pop rcx
	pop rbx
	pop rax
%endmacro

%macro pushStateExceptRAX 0
    push rbx
    push rcx
    push rdx
    push rbp
    push rdi
    push rsi
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15
%endmacro

%macro popStateExceptRAX 0
    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rsi
    pop rdi
    pop rbp
    pop rdx
    pop rcx
    pop rbx
%endmacro

%macro saveSnapshot 0
    ;especiales
    mov rax, [rsp]           ; RIP
    mov [snapshot+8*16], rax
    mov rax, [rsp+8]         ; CS
    mov [snapshot+8*17], rax
    mov rax, [rsp+16]        ; RFLAGS
    mov [snapshot+8*18], rax

    ; ¿hay user RSP/SS?
    mov rbx, [rsp+8]
    and rbx, 0x3
    cmp rbx, 3
    jne .no_user
    mov rax, [rsp+24]        ; user RSP
    mov [snapshot+8*19], rax
    mov rax, [rsp+32]        ; user SS
    mov [snapshot+8*20], rax
.no_user:

    ;generales
    mov [snapshot+8*0],  rax   ; RAX (usa rax actual)∫
    mov rax, rbx
    mov [snapshot+8*1],  rax   ; RBX
    mov rax, rcx
    mov [snapshot+8*2],  rax
    mov rax, rdx
    mov [snapshot+8*3],  rax
    mov rax, rsi
    mov [snapshot+8*4],  rax
    mov rax, rdi
    mov [snapshot+8*5],  rax
    mov rax, rbp
    mov [snapshot+8*6],  rax
    mov rax, r8
    mov [snapshot+8*7],  rax
    mov rax, r9
    mov [snapshot+8*8],  rax
    mov rax, r10
    mov [snapshot+8*9],  rax
    mov rax, r11
    mov [snapshot+8*10], rax
    mov rax, r12
    mov [snapshot+8*11], rax
    mov rax, r13
    mov [snapshot+8*12], rax
    mov rax, r14
    mov [snapshot+8*13], rax
    mov rax, r15
    mov [snapshot+8*14], rax
    mov rax, rsp
    mov [snapshot+8*15], rax
%endmacro


%macro irqHandlerMaster 1
	saveSnapshot
	pushState

	mov rdi, %1 ; pasaje de parametro
	call irqDispatcher

	; signal pic EOI (End of Interrupt)
	mov al, 20h
	out 20h, al

	popState
	iretq
%endmacro

%macro syscallHandler 0
    pushStateExceptRAX

    call syscallDispatcher
	
    popStateExceptRAX
    iretq
%endmacro


%macro exceptionHandler 1
	saveSnapshot

    pushState
    mov rdi, %1
    call exceptionDispatcher
    popState

    call getStackBase
    mov rsp, rax

    pushfq
    pop rax
    or rax, 0x200         ; habilitar IF
    push rax              ; flags
    push qword 0x08       ; cs
    push qword 0x400000
    iretq
%endmacro



_hlt:
	sti
	hlt
	ret

_cli:
	cli
	ret


_sti:
	sti
	ret

picMasterMask:
	push rbp
    mov rbp, rsp
    mov ax, di
    out	21h,al
    pop rbp
    retn

picSlaveMask:
	push    rbp
    mov     rbp, rsp
    mov     ax, di  ; ax = mascara de 16 bits
    out	0A1h,al
    pop     rbp
    retn


;8254 Timer (Timer Tick)
_irq00Handler:
	irqHandlerMaster 0

;Keyboard
_irq01Handler:
	irqHandlerMaster 1 ; macro que llama a irqDispatcher(1)

;Cascade pic never called
_irq02Handler:
	irqHandlerMaster 2

;Serial Port 2 and 4
_irq03Handler:
	irqHandlerMaster 3

;Serial Port 1 and 3
_irq04Handler:
	irqHandlerMaster 4

;USB
_irq05Handler:
	irqHandlerMaster 5

;syscalls
_int80Handler:
    syscallHandler

;Zero Division Exception
_exception0Handler:
	exceptionHandler 0

;invalid opcode
_exception6Handler:
	exceptionHandler 6

haltcpu:
	cli
	hlt
	ret

SECTION .bss
	aux resq 1
	global snapshot
	snapshot  resq 21
