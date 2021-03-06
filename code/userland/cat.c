#include "syscall.h"
#include "lib.c"

#define ARGC_ERROR    "Error: missing argument.\n"
#define OPEN_ERROR  "Error: could not open file.\n"

int
main(int argc, char *argv[])
{
  if (argc < 2) {
      puts2(ARGC_ERROR);
      Exit(1);
  }
    
  int success = 1;
    
  OpenFileId id = Open(argv[1]);

  if (id < 0) {
    puts2(OPEN_ERROR);
    return -1;
  } else {
    char buffer[1];
    while (Read(buffer, 1, id) > 0) {
       Write(buffer, 1, CONSOLE_OUTPUT);
    }
    if (buffer[1] != '\n') {
      Write("\n", 1, CONSOLE_OUTPUT);
    }
    Close(id);
  }
  
  return success;
}