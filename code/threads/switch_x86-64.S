/// Machine-dependent context switch routines for x86_64.
///
/// Do not modify these unless you know what you are doing.  You would break
/// context switches otherwise.
///
/// Copyright (c) 1992-1993 The Regents of the University of California.
///               2016-2021 Docentes de la Universidad Nacional de Rosario.
/// All rights reserved.  See copyright.h for copyright notice and limitation
/// of liability and disclaimer of warranty provisions.


#include "switch.h"


        .text
        .align  8

/// Expects the following registers to be initialized:
///
/// * `rax` -- points to startup function (interrupt enable).
/// * `rbx` -- contains initial argument to thread function [`InitialArg`].
/// * `rsi` -- points to thread function [`InitialPC`].
/// * `rdi` -- points to `Thread::Finish` [`WhenDonePCState`].
///
/// This code takes into account that the stack must be aligned to 16 bytes
/// when calling a function.
        .globl  ThreadRoot
ThreadRoot:
        push   %rdi
        push   %rsi
        callq  *%rax  // StartupPC()
        mov    %rbx, %rdi
        mov    (%rsp), %rsi
        callq  *%rsi  // InitialPC(InitialArg)
        pop    %rsi
        pop    %rsi
        callq  *%rsi  // WhenDonePC()
        // NOT REACHED.
        ret

/// void SWITCH(Thread *t1, Thread *t2)
///
/// On entry, this is the state of stack and registers (see `abi.pdf` in the
/// `http://x86-64.org` site):
///     rdi    ->  Thread *t1
///     rsi    ->  Thread *t2
///     (rsp)  ->  return address
        .globl  SWITCH
SWITCH:
        mov    %rax, _RAX(%rdi)  // Save registers.
        mov    %rbx, _RBX(%rdi)
        mov    %rcx, _RCX(%rdi)
        mov    %rdx, _RDX(%rdi)
        mov    %rsi, _RSI(%rdi)
        mov    %rdi, _RDI(%rdi)
        mov    %rbp, _RBP(%rdi)
        mov    %rsp, _RSP(%rdi)  // Save stack pointer.
        movq   0(%rsp), %rax     // Get return address from stack into `rax`.
        mov    %rax, _PC(%rdi)   // Save it into the PC storage.
        mov    %r8, _R8(%rdi)
        mov    %r9, _R9(%rdi)
        mov    %r10, _R10(%rdi)
        mov    %r11, _R11(%rdi)
        mov    %r12, _R12(%rdi)
        mov    %r13, _R13(%rdi)
        mov    %r14, _R14(%rdi)
        mov    %r15, _R15(%rdi)

        mov    _RBX(%rsi), %rbx  // Restore old registers.
        mov    _RCX(%rsi), %rcx
        mov    _RDX(%rsi), %rdx
        mov    _RDI(%rsi), %rdi
        mov    _RBP(%rsi), %rbp
        mov    _RSP(%rsi), %rsp  // Restore stack pointer.
        mov    _R8(%rsi), %r8
        mov    _R9(%rsi), %r9
        mov    _R10(%rsi), %r10
        mov    _R11(%rsi), %r11
        mov    _R12(%rsi), %r12
        mov    _R13(%rsi), %r13
        mov    _R14(%rsi), %r14
        mov    _R15(%rsi), %r15
        mov    _PC(%rsi), %rax   // Restore return address into `eax`.
        mov    %rax, 0(%rsp)     // Copy over the ret address on the stack.
        mov    %rsi, %rax
        mov    _RSI(%rsi), %rsi  // Replace `rsi` with stored value.
        mov    _RAX(%rax), %rax  // Use `t2` pointer previously saved in
                                 // `rax`.

        ret
