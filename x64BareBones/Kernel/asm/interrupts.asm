
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
%define DELTA 120

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
    ; 1) RAX…RDX
    mov   rax, [rsp+112]        ; RAX
    mov   [snapshot+  0], rax
    mov   rax, [rsp+104]        ; RBX
    mov   [snapshot+  8], rax
    mov   rax, [rsp+ 96]        ; RCX
    mov   [snapshot+ 16], rax
    mov   rax, [rsp+ 88]        ; RDX
    mov   [snapshot+ 24], rax

    ; 2) RSI, RDI, RBP
    mov   rax, [rsp+ 64]        ; RSI
    mov   [snapshot+ 32], rax
    mov   rax, [rsp+ 72]        ; RDI
    mov   [snapshot+ 40], rax
    mov   rax, [rsp+ 80]        ; RBP
    mov   [snapshot+ 48], rax

    ; 3) R8…R15
    mov   rax, [rsp+ 56]        ; R8
    mov   [snapshot+ 56], rax
    mov   rax, [rsp+ 48]        ; R9
    mov   [snapshot+ 64], rax
    mov   rax, [rsp+ 40]        ; R10
    mov   [snapshot+ 72], rax
    mov   rax, [rsp+ 32]        ; R11
    mov   [snapshot+ 80], rax
    mov   rax, [rsp+ 24]        ; R12
    mov   [snapshot+ 88], rax
    mov   rax, [rsp+ 16]        ; R13
    mov   [snapshot+ 96], rax
    mov   rax, [rsp+  8]        ; R14
    mov   [snapshot+104], rax
    mov   rax, [rsp    ]        ; R15
    mov   [snapshot+112], rax

    ; -- special registers --
    mov   rbx, [rsp+DELTA+8]    ; CS
    mov   [snapshot+136], rbx
    mov   rax, [rsp+DELTA+16]   ; RFLAGS
    mov   [snapshot+144], rax
    mov   rax, [rsp+DELTA]      ; RIP
    mov   [snapshot+128], rax

    ; Determine if coming from user mode
    mov   rax, rbx
    and   rax, 3
    cmp   rax, 3
    jne   .no_user

    mov   rax, [rsp+DELTA+24]   ; user RSP
    mov   [snapshot+120], rax   ; RSP
    mov   [snapshot+152], rax   ; URSP
    mov   rax, [rsp+DELTA+32]   ; user SS
    mov   [snapshot+160], rax   ; USS
    jmp   .done

.no_user:
    lea   rax, [rsp+DELTA+24]   ; RSP before pushState
    mov   [snapshot+120], rax
    mov   qword [snapshot+152], 0
    mov   qword [snapshot+160], 0

.done:
%endmacro

%macro irqHandlerMaster 1
	pushState
	saveSnapshot

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
    pushState
	saveSnapshot
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
