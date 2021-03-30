/// Definitions needed for implementing context switching.
///
/// Context switching is inherently machine dependent, since the registers to
/// be saved, how to set up an initial call frame, etc, are all specific to a
/// processor architecture.
///
/// This file currently supports the following architectures:
/// * i386 (32-bit Intel)
/// * x86_64 (64-bit Intel)
///
/// Copyright (c) 1992-1993 The Regents of the University of California.
///               2016-2021 Docentes de la Universidad Nacional de Rosario.
/// All rights reserved.  See copyright.h for copyright notice and limitation
/// of liability and disclaimer of warranty provisions.

#ifndef NACHOS_THREADS_SWITCH__H
#define NACHOS_THREADS_SWITCH__H


#if defined(HOST_i386)
    #include "switch_i386.h"
#elif defined(HOST_x86_64)
    #include "switch_x86-64.h"
#else
    #error "Unknown CPU architecture."
#endif


#endif
