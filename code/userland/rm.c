#include "syscall.h"
#include "lib.c"

#define ARGC_ERROR    "Error: missing argument.\n"
#define REMOVE_ERROR  "Error: could not remove file.\n"

int
main(int argc, char *argv[])
{
    if (argc < 2) {
        puts2(ARGC_ERROR);
        Exit(1);
    }
    
    int success = 1;
    for (unsigned i = 1; i < argc; i++) {
        if (Remove(argv[i]) < 0) {
            puts2(REMOVE_ERROR);
            success = 0;
        }
    }
    return !success;
}