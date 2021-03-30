/// Data structures defining the Nachos Object Code Format
///
/// Basically, we only know about three types of segments: code (read-only),
/// initialized data, and unitialized data.
///
/// Copyright (c) 1992-1993 The Regents of the University of California.
///               2016-2021 Docentes de la Universidad Nacional de Rosario.
/// All rights reserved.  See `copyright.h` for copyright notice and
/// limitation of liability and disclaimer of warranty provisions.

#ifndef NACHOS_BIN_NOFF__H
#define NACHOS_BIN_NOFF__H


#include <stdint.h>


#define NOFF_MAGIC  0xBADFAD  // Magic number denoting Nachos object code
                              // file.

typedef struct noffSegment {
    uint32_t virtualAddr;  // Location of segment in virtual address space.
    uint32_t inFileAddr;   // Location of segment in this file.
    uint32_t size;         // Size of segment.
} noffSegment;

typedef struct noffHeader {
    uint32_t noffMagic;      // Should be `NOFF_MAGIC`.
    noffSegment code;        // Executable code segment.
    noffSegment initData;    // Initialized data segment.
    noffSegment uninitData;  // Uninitialized data segment -- should be
                             // zeroed before use.
} noffHeader;


#endif
