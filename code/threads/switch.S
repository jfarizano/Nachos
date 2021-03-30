/// Machine-dependent context switch routines.
///
/// Do not modify these unless you know what you are doing.  You would break
/// context switches otherwise.
///
/// Context switching is inherently machine-dependent, since the registers to
/// be saved, how to set up an initial call frame, etc., are all specific to
/// a processor architecture.
///
/// This file currently supports the following architectures:
/// * i386 (32-bit Intel)
/// * x86_64 (64-bit Intel)
///
/// We define two routines for each architecture:
///
/// 1. `void ThreadRoot(InitialPC, InitialArg, WhenDonePC, StartupPC)`
///
///    Parameters:
///    * `InitialPC` is the program counter of the procedure to run in this
///      in this thread.
///    * `InitialArg` is the single argument to the thread.
///    * `WhenDonePC` is the routine to call when the thread returns.
///    * `StartupPC` is the routine to call when the thread is started.
///
///    Called from the `SWITCH` routine to start a thread for the first time.
///
/// 2. `void SWITCH(oldThread, newThread)`
///
///    Parameters:
///    * `oldThread` is the current thread that was running, where the CPU
///      register state is to be saved.
///    * `newThread` is the new thread to be run, where the CPU register
///      state is to be loaded from.
///
/// Copyright (c) 1992-1993 The Regents of the University of California.
///               2016-2021 Docentes de la Universidad Nacional de Rosario.
/// All rights reserved.  See copyright.h for copyright notice and limitation
/// of liability and disclaimer of warranty provisions.


#if defined(HOST_i386)
    #include "switch_i386.S"
#elif defined(HOST_x86_64)
    #include "switch_x86-64.S"
#else
    #error "Unknown CPU architecture."
#endif
