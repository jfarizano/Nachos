/// Definitions needed for implementing context switching in x86_64.
///
/// Copyright (c) 1992-1993 The Regents of the University of California.
///               2016-2021 Docentes de la Universidad Nacional de Rosario.
/// All rights reserved.  See copyright.h for copyright notice and limitation
/// of liability and disclaimer of warranty provisions.


/// The offsets of the registers from the beginning of the thread object.
#define _RSP    0
#define _RAX    8
#define _RBX   16
#define _RCX   24
#define _RDX   32
#define _RBP   40
#define _RSI   48
#define _RDI   56
#define _PC    64
#define _R8    72
#define _R9    80
#define _R10   88
#define _R11   96
#define _R12  104
#define _R13  112
#define _R14  120
#define _R15  128

/// These definitions are used in `Thread::StackAllocate`.
#define PCState          (_PC  / 8 - 1)
#define FPState          (_RBP / 8 - 1)
#define InitialPCState   (_RSI / 8 - 1)
#define InitialArgState  (_RBX / 8 - 1)
#define WhenDonePCState  (_RDI / 8 - 1)
#define StartupPCState   (_RAX / 8 - 1)
