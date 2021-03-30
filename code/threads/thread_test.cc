/// Simple test case for the threads assignment.
///
/// Create several threads, and have them context switch back and forth
/// between themselves by calling `Thread::Yield`, to illustrate the inner
/// workings of the thread system.
///
/// Copyright (c) 1992-1993 The Regents of the University of California.
///               2007-2009 Universidad de Las Palmas de Gran Canaria.
///               2016-2021 Docentes de la Universidad Nacional de Rosario.
/// All rights reserved.  See `copyright.h` for copyright notice and
/// limitation of liability and disclaimer of warranty provisions.


#include "thread_test_garden.hh"
#include "thread_test_prod_cons.hh"
#include "thread_test_simple.hh"
#include "lib/utility.hh"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


typedef struct {
    void      (*func)();
    const char *name;
    const char *description;
} Test;

static const Test TESTS[] = {
    { &ThreadTestSimple,   "simple",   "Simple thread interleaving" },
    { &ThreadTestGarden,   "garden",   "Ornamental garden" },
    { &ThreadTestProdCons, "prodcons", "Producer/Consumer" }
};
static const unsigned NUM_TESTS = sizeof TESTS / sizeof TESTS[0];

static const unsigned NAME_MAX_LEN = 32;

static bool
Parse(char *choice, unsigned *index)
{
    ASSERT(choice != nullptr);
    ASSERT(index != nullptr);

    // Remove trailing newline.
    unsigned len = strlen(choice);
    if (choice[len - 1] == '\n') {
        choice[len - 1] = '\0';
    }

    // Skip empty lines.
    if (choice[0] == '\0') {
        return false;
    }

    // Try an integer representing a test index.
    char *end_p;
    unsigned n = strtoul(choice, &end_p, 10);
    if (end_p[0] == '\0') {
        if (n < NUM_TESTS) {
            *index = n;
            return true;
        } else {
            return false;
        }
    }

    // Try a string naming a test.
    for (unsigned i = 0; i < NUM_TESTS; i++) {
        const Test *t = &TESTS[i];
        if (strcmp(choice, t->name) == 0) {
            *index = i;
            return true;
        }
    }

    return false;
}

/// Ask the user interactively to choose a test.
static unsigned
Choose()
{
    printf("Available tests:\n");
    for (unsigned i = 0; i < NUM_TESTS; i++) {
        const Test *t = &TESTS[i];
        printf("(%u) %s: %s\n", i, t->name, t->description);
    }

    char choice[NAME_MAX_LEN];
    unsigned index;
    do {
        printf("Choose a test to run: ");
        fflush(stdout);
        ASSERT(fgets(choice, sizeof choice, stdin) != nullptr);
    } while (!Parse(choice, &index));

    return index;
}

static void
Run(unsigned i)
{
    ASSERT(i < NUM_TESTS);

    const Test *t = &TESTS[i];
    printf("\nRunning thread test %u: %s -- %s.\n",
           i, t->name, t->description);
    (*TESTS[i].func)();
    printf("\n");
}

void
ThreadTest()
{
    DEBUG('t', "Entering thread test\n");

    const unsigned i = Choose();
    Run(i);
}
