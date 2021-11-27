/// Copyright (c) 1992-1993 The Regents of the University of California.
///               2016-2021 Docentes de la Universidad Nacional de Rosario.
/// All rights reserved.  See `copyright.h` for copyright notice and
/// limitation of liability and disclaimer of warranty provisions.


#include "assert.hh"

#include <stdio.h>
#include <stdlib.h>


void
Assert(bool result, const char *expString, const char *filename,
       unsigned line)
{
    if (!result) {
        fprintf(stderr, "\nAssertion failed!\n"
                        "\tExpression: `%s`\n"
                        "\tLocation: file `%s`, line %u\n",
                expString, filename, line);
        fflush(stderr);
        abort();
    }
}
