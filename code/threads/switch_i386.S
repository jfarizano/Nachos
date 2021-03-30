/// Machine-dependent context switch routines for i386.
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
        .align  2

/// Expects the following registers to be initialized:
///
/// * `eax` -- points to startup function (interrupt enable).
/// * `edx` -- contains inital argument to thread function.
/// * `esi` -- points to thread function.
/// * `edi` -- points to `Thread::Finish`.
        .globl  ThreadRoot
ThreadRoot:
        pushl  %edx   // InitialArg
        call   *%ecx  // StartupPC
        call   *%esi  // InitialPC
        call   *%edi  // WhenDonePC
        // NOT REACHED.
        ret

/// void SWITCH(Thread *t1, Thread *t2)
///
/// On entry, stack looks like this:
///     8(esp)  ->  Thread *t2
///     4(esp)  ->  Thread *t1
///      (esp)  ->  return address
///
/// We push the current `eax` on the stack so that we can use it as
/// a pointer to `t1`, this decrements `esp` by 4, so when we use it
/// to reference stuff on the stack, we add 4 to the offset.
        .comm   _eax_save, 4
        .globl  SWITCH
SWITCH:
        movl  %eax, _eax_save   // Save the value of eax.
        movl  4(%esp), %eax     // Move pointer to `t1` into `eax`.
        movl  %ebx, _EBX(%eax)  // Save registers.
        movl  %ecx, _ECX(%eax)
        movl  %edx, _EDX(%eax)
        movl  %esi, _ESI(%eax)
        movl  %edi, _EDI(%eax)
        movl  %ebp, _EBP(%eax)
        movl  %esp, _ESP(%eax)  // Save stack pointer.
        movl  _eax_save, %ebx   // Get the saved value of `eax`.
        movl  %ebx, _EAX(%eax)  // Store it.
        movl  0(%esp), %ebx     // Get return address from stack into `ebx`.
        movl  %ebx, _PC(%eax)   // save it into the PC storage.

        movl  8(%esp), %eax     // Move pointer to `t2` into `eax`.

        movl  _EAX(%eax), %ebx  // Get new value for `eax` into `ebx`.
        movl  %ebx, _eax_save   // Save it.
        movl  _EBX(%eax), %ebx  // Retore old registers.
        movl  _ECX(%eax), %ecx
        movl  _EDX(%eax), %edx
        movl  _ESI(%eax), %esi
        movl  _EDI(%eax), %edi
        movl  _EBP(%eax), %ebp
        movl  _ESP(%eax), %esp  // Restore stack pointer.
        movl  _PC(%eax), %eax   // Restore return address into `eax`.
        movl  %eax, 0(%esp)     // Copy over the ret address on the stack.
        movl  _eax_save, %eax

        ret
