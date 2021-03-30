/// Miscellaneous useful definitions, including debugging utilities.
///
/// See also `lib/debug.hh`.
///
/// Copyright (c) 1992-1993 The Regents of the University of California.
///               2016-2021 Docentes de la Universidad Nacional de Rosario.
/// All rights reserved.  See `copyright.h` for copyright notice and
/// limitation of liability and disclaimer of warranty provisions.

#ifndef NACHOS_LIB_UTILITY__HH
#define NACHOS_LIB_UTILITY__HH


#include "assert.hh"
#include "debug.hh"


/// Useful definitions for diverse data structures.

const unsigned BITS_IN_BYTE = 8;
const unsigned BITS_IN_WORD = 32;


/// Miscellaneous useful routines.

//#define min(a, b)  (((a) < (b)) ? (a) : (b))
//#define max(a, b)  (((a) > (b)) ? (a) : (b))

/// Divide and either round up or down.

template <typename T>
inline T
DivRoundDown(T n, T s)
{
    return n / s;
}

template <typename T>
inline T
DivRoundUp(T n, T s)
{
    return n / s + (n % s > 0 ? 1 : 0);
}

/// This declares the type `VoidFunctionPtr` to be a “pointer to a
/// function taking a pointer argument and returning nothing”.
///
/// With such a function pointer (say it is "func"), we can call it like
/// this:
///    func (arg);
/// or:
///    (*func) (arg);
///
/// This is used by `Thread::Fork` and for interrupt handlers, as well as a
/// couple of other places.
typedef void (*VoidFunctionPtr)(void *arg);

typedef void (*VoidNoArgFunctionPtr)();


// Include interface that isolates us from the host machine system library.
// Requires definition of `VoidFunctionPtr`.
#include "machine/system_dep.hh"


/// Global object for debug output.
extern Debug debug;

#define DEBUG(...)  (debug.Print)(__FILE__, __LINE__, __func__, __VA_ARGS__)
#define DEBUG_CONT  (debug.PrintCont)


#endif
