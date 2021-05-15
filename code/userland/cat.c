#include "syscall.h"
#include "lib.c"

#define ARGC_ERROR    "Error: missing argument.\n"
#define OPEN_ERROR  "Error: could not open file.\n"

int
main(int argc, char *argv[])
{
  if (argc < 2) {
      puts(ARGC_ERROR);
      Exit(1);
  }
    
  int success = 1;
    
  OpenFileId id = Open(argv[1]);

  if (id < 0) {
    puts(OPEN_ERROR);
    return -1;
  } else {
    char *buffer;
    while (Read(buffer, 1, id)) {
       Write(buffer, 1, CONSOLE_OUTPUT);
    }
  }
  
  return success;
}