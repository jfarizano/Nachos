/// Copyright (c) 1992-1993 The Regents of the University of California.
///               2007-2009 Universidad de Las Palmas de Gran Canaria.
///               2016-2021 Docentes de la Universidad Nacional de Rosario.
/// All rights reserved.  See `copyright.h` for copyright notice and
/// limitation of liability and disclaimer of warranty provisions.


#include "thread_test_simple.hh"
#include "system.hh"
#include "lib/utility.hh"

#ifdef SEMAPHORE_TEST
#include "semaphore.hh"
Semaphore *SEMAPHORE = new Semaphore("Test", 3);
#endif

#include <stdio.h>
#include <string.h>

static const unsigned NUM_THREADS = 5;

/// Loop 10 times, yielding the CPU to another ready thread each iteration.
///
/// * `name` points to a string with a thread name, just for debugging
///   purposes.
void
SimpleThread(void *name_)
{
    // Reinterpret arg `name` as a string.
    char *name = (char *) name_;

    // If the lines dealing with interrupts are commented, the code will
    // behave incorrectly, because printf execution may cause race
    // conditions.
    for (unsigned num = 0; num < 10; num++) {
        #ifdef SEMAPHORE_TEST
            DEBUG('s', "Thread %s doing P\n", name);
            SEMAPHORE->P();
        #endif
        printf("*** Thread `%s` is running: iteration %u\n", name, num);
        #ifdef SEMAPHORE_TEST
            DEBUG('s', "Thread %s doing V\n", name);
            SEMAPHORE->V();
        #endif
        currentThread->Yield();
    }
    printf("!!! Thread `%s` has finished\n", name);
}

/// Set up a ping-pong between several threads.
///
/// Do it by launching one thread which calls `SimpleThread`, and finally
/// calling `SimpleThread` on the current thread.
void
ThreadTestSimple()
{
    for (unsigned i = 0; i < NUM_THREADS - 1; i++) {
        char *name = new char [64];
        sprintf(name, "%u", i + 1);
        Thread *newThread = new Thread(name, false, 2);
        newThread->Fork(SimpleThread, (void *) name);
    }

    SimpleThread((void *) "0");
}
