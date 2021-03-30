/// Lock/Mutex, a synchronization primitive
///
/// A data structure for synchronizing threads.
///
/// Base Nachos only provides the interface for this primitive.  In order to
/// use it, an implementation has to be written.
///
/// All synchronization objects have a `name` parameter in the constructor;
/// its only aim is to ease debugging the program.
///
/// Copyright (c) 1992-1993 The Regents of the University of California.
///               2000      José Miguel Santos Espino - ULPGC.
///               2016-2021 Docentes de la Universidad Nacional de Rosario.
/// All rights reserved.  See `copyright.h` for copyright notice and
/// limitation of liability and disclaimer of warranty provisions.

#ifndef NACHOS_THREADS_LOCK__HH
#define NACHOS_THREADS_LOCK__HH


/// This class defines a “lock”.
///
/// A lock can have two states: free and busy. Only two operations are
/// allowed on locks:
///
/// * `Acquire` -- wait until the lock is free and mark is as busy.
/// * `Release` -- mark the lock as free, thereby awakening some other thread
///   that were blocked on an `Acquired`.
///
/// For convenience, nobody but the thread that holds the lock can free it.
/// There is no operation for reading the state of the lock.
class Lock {
public:

    /// Constructor: set up the lock as free.
    Lock(const char *debugName);

    ~Lock();

    /// For debugging.
    const char *GetName() const;

    /// Operations on the lock.
    ///
    /// Both must be *atomic*.
    void Acquire();
    void Release();

    /// Returns `true` if the current thread is the one that possesses the
    /// lock.
    ///
    /// Useful for checks in `Release` and in condition variables.
    bool IsHeldByCurrentThread() const;

private:

    /// For debugging.
    const char *name;

    // Add other needed fields here.
};


#endif
