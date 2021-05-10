#ifndef NACHOS_FILESYS_SYNCHCONSOLE__HH
#define NACHOS_FILESYS_SYNCHCONSOLE__HH

#include "machine/console.hh"
#include "threads/lock.hh"
#include "threads/semaphore.hh"

class SynchConsole {
public:

  SynchConsole(const char *name);

  ~SynchConsole();

  // Read char from console
  void ReadChar(char *c);

  // Write char to console
  void WriteChar(const char *c);

  // Read buffer from console
  void ReadBuffer(char *buffer, unsigned size);

  // Write buffer to console 
  void WriteBuffer(char *buffer, unsigned size);

  // Used by console to signal I/O completion.
  void ReadAvail();
  void WriteDone();

private:
  Console *console;
  Semaphore *readAvail;
  Semaphore *writeDone;
  Lock *readLock;  
  Lock *writeLock;
  
};

#endif