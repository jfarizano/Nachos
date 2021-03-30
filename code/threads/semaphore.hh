/// Semaphore, a synchronization primitive
///
/// A data structure for synchronizing threads.
///
/// This is the only synchronization primitive that comes implemented in
/// Nachos.  For the other two, locks and condition variables, only the
/// interface is provided; an implementation has to be written in order to
/// be able to use them.
///
/// All synchronization objects have a `name` parameter in the constructor;
/// its only aim is to ease debugging the program.
///
/// Copyright (c) 1992-1993 The Regents of the University of California.
///               2000      José Miguel Santos Espino - ULPGC.
///               2016-2021 Docentes de la Universidad Nacional de Rosario.
/// All rights reserved.  See `copyright.h` for copyright notice and
/// limitation of liability and disclaimer of warranty provisions.
//
#ifndef NACHOS_THREADS_SEMAPHORE__HH
#define NACHOS_THREADS_SEMAPHORE__HH


#include "thread.hh"
#include "lib/list.hh"


/// This class defines a “semaphore”, which has a positive integer as its
/// value.
///
/// Semaphores offer only two operations:
///
/// * `P` -- wait until `value > 0`, then decrement `value`.
/// * `V` -- increment `value`, awaken a waiting thread if any.
///
/// Observe that this interface does *not* allow to read the semaphore value
/// directly -- even if you were able to read it, it would serve for nothing,
/// because meanwhile another thread could have modified the semaphore, in
/// case you have lost the CPU for some time.
class Semaphore {
public:

    /// Constructor: give an initial value to the semaphore.
    ///
    /// Set initial value.
    Semaphore(const char *debugName, int initialValue);

    ~Semaphore();

    /// For debugging.
    const char *GetName() const;

    /// The only public operations on the semaphore.
    ///
    /// Both of them must be *atomic*.
    void P();
    void V();

private:

    /// For debugging.
    const char *name;

    /// Semaphore value, it is always `>= 0`.
    int value;

    /// Queue of threads waiting on `P` because the value is zero.
    List<Thread *> *queue;

};


#endif
