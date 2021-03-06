/// Copyright (c) 1992-1993 The Regents of the University of California.
///               2007-2009 Universidad de Las Palmas de Gran Canaria.
///               2016-2021 Docentes de la Universidad Nacional de Rosario.
/// All rights reserved.  See `copyright.h` for copyright notice and
/// limitation of liability and disclaimer of warranty provisions.


#include "thread_test_garden.hh"
#include "system.hh"

#include <stdio.h>

#ifdef SEMAPHORE_TEST
#include "semaphore.hh"
Semaphore *SEMAPHORE_TURNS = new Semaphore("Test", 1);
#endif

static const unsigned NUM_TURNSTILES = 5;
static const unsigned ITERATIONS_PER_TURNSTILE = 3;
// static bool done[NUM_TURNSTILES];
static int count;

static void
Turnstile(void *n_)
{
    unsigned *n = (unsigned *) n_;

    for (unsigned i = 0; i < ITERATIONS_PER_TURNSTILE; i++) {
        #ifdef SEMAPHORE_TEST
        DEBUG('s', "Turnstile %u doing P\n", *n);
        SEMAPHORE_TURNS->P();
        #endif
        int temp = count;
        count = temp + 1;
        #ifdef SEMAPHORE_TEST
        DEBUG('s', "Turnstile %u doing V\n", *n);
        SEMAPHORE_TURNS->V();
        #endif
        currentThread->Yield();
    }
    printf("Turnstile %u finished. Count is now %u.\n", *n, count);
    delete n;
    currentThread->Finish(0);
}

void
ThreadTestGarden()
{
    List<Thread *> *threads = new List<Thread *>;
    // Launch a new thread for each turnstile.
    for (unsigned i = 0; i < NUM_TURNSTILES; i++) {
        printf("Launching turnstile %u.\n", i);
        char name[16];
        sprintf(name, "Turnstile %u", i);
        unsigned *n = new unsigned;
        *n = i;
        Thread *t = new Thread(name, true, 2);
        t->Fork(Turnstile, (void *) n);
        threads->Append(t);
    }

    Thread *t;
    while (!threads->IsEmpty()) {
        t = threads->Pop();
        t->Join();
    }
    delete threads;
    #ifdef SEMAPHORE_TEST
        delete SEMAPHORE_TURNS;
    #endif
    printf("All turnstiles finished. Final count is %u (should be %u).\n",
           count, ITERATIONS_PER_TURNSTILE * NUM_TURNSTILES);
}
