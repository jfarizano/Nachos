#include "syscall.h"
#include "lib.c"

#define ARGC_ERROR    "Error: missing argument.\n"
#define OPEN_ERROR  "Error: could not open or create file.\n"

int
main(int argc, char *argv[])
{
  if (argc < 3) {
      puts2(ARGC_ERROR);
      Exit(1);
  }
    
  int success = 1;
    
  OpenFileId inputFile = Open(argv[1]);
  Create(argv[2]);
  OpenFileId outputFile = Open(argv[2]);

  if (inputFile < 0 || outputFile < 0) {
    puts2(OPEN_ERROR);
    return -1;
  } else {
    char buffer[1];
    
    while (Read(buffer, 1, inputFile) > 0) {
       Write(buffer, 1, outputFile);
    }
    Close(inputFile);
    Close(outputFile);
  }
  
  return success;
}