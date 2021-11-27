/// Copyright (c) 2016-2021 Docentes de la Universidad Nacional de Rosario.
/// All rights reserved.  See `copyright.h` for copyright notice and
/// limitation of liability and disclaimer of warranty provisions.


#include "transfer.hh"
#include "machine/machine.hh"
#include "threads/system.hh"

#include <string.h>


static const unsigned MAX_ARG_COUNT  = 32;
static const unsigned MAX_ARG_LENGTH = 128;

#ifdef USE_TLB
static const int MAX_MEM_TRIES = 4;
#else
static const int MAX_MEM_TRIES = 1;
#endif

/// Count the number of arguments up to a null (which is not counted).
///
/// Returns true if the number fit in the established limits and false if
/// too many arguments were provided.
static inline
bool CountArgsToSave(int address, unsigned *count)
{
    ASSERT(address != 0);
    ASSERT(count != nullptr);

    int val;
    unsigned c = 0;
    do {
        int tries = 0;
        for (; tries < 4 && !machine->ReadMem(address + 4 * c, 4, &val); tries++){};
        if (tries == 4) {
            ASSERT(false);
        }
        c++;
    } while (c < MAX_ARG_COUNT && val != 0);
    if (c == MAX_ARG_COUNT && val != 0) {
        // The maximum number of arguments was reached but the last is not
        // null.
        return false;
    }

    *count = c - 1;
    return true;
}

char **
SaveArgs(int address)
{
    ASSERT(address != 0);

    unsigned count;
    if (!CountArgsToSave(address, &count)) {
        return nullptr;
    }

    DEBUG('e', "Saving %u command line arguments from parent process.\n",
          count);

    // Allocate an array of `count` pointers.  We know that `count` will
    // always be at least 1.
    char **args = new char * [count + 1];

    for (unsigned i = 0; i < count; i++) {
        args[i] = new char [MAX_ARG_LENGTH];
        int strAddr;
        // For each pointer, read the corresponding string.
        int tries = 0;
        for (; tries < 4 && !machine->ReadMem(address + i * 4, 4, &strAddr); tries++){};
        if (tries == 4) {
            ASSERT(false);
        }
        ReadStringFromUser(strAddr, args[i], MAX_ARG_LENGTH);
    }
    args[count] = nullptr;  // Write the trailing null.

    return args;
}

unsigned
WriteArgs(char **args)
{
    ASSERT(args != nullptr);

    DEBUG('e', "Writing command line arguments into child process.\n");

    // Start writing the strings where the current SP points.  Write them in
    // reverse order (i.e. the string from the first argument will be in a
    // higher memory address than the string from the second argument).
    int argsAddress[MAX_ARG_COUNT];
    unsigned c;
    int sp = machine->ReadRegister(STACK_REG);
    for (c = 0; c < MAX_ARG_COUNT; c++) {
        if (args[c] == nullptr) {   // If the last was reached, terminate.
            break;
        }
        sp -= strlen(args[c]) + 1;  // Decrease SP (leave one byte for \0).
        WriteStringToUser(args[c], sp);  // Write the string there.
        argsAddress[c] = sp;        // Save the argument's address.
        delete args[c];             // Free the string.
    }
    delete args;  // Free the array.
    ASSERT(c < MAX_ARG_COUNT);

    sp -= sp % 4;     // Align the stack to a multiple of four.
    sp -= c * 4 + 4;  // Make room for `argv`, including the trailing null.
    // Write each argument's address.
    for (unsigned i = 0; i < c; i++) {
        int tries = 0;
        for (; tries < 4 && !machine->WriteMem(sp + 4 * i, 4, argsAddress[i]); tries++){};
        if (tries == 4) {
            ASSERT(false);
        }
    }
    int tries = 0;
    for (; tries < MAX_MEM_TRIES && !machine->WriteMem(sp + 4 * c, 4, 0); tries++){};  // The last is null.
    if (tries == MAX_MEM_TRIES) {
        ASSERT(false);
    }

    machine->WriteRegister(STACK_REG, sp);
    return c;
}
