/// Copyright (c) 1992-1993 The Regents of the University of California.
///               2016-2021 Docentes de la Universidad Nacional de Rosario.
/// All rights reserved.  See `copyright.h` for copyright notice and
/// limitation of liability and disclaimer of warranty provisions.

#ifndef NACHOS_LIB_ASSERT__HH
#define NACHOS_LIB_ASSERT__HH


/// If `condition` is false, print a message and dump core.
///
/// Useful for documenting assumptions in the code.
///
/// NOTE: needs to be a macro, so as to be able to print both the condition
/// as a string and the location where the error occurred.
#define ASSERT(condition)  Assert(condition, #condition, __FILE__, __LINE__)

void Assert(bool result, const char *expString, const char *filename,
            unsigned line);


#endif
