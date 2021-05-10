#ifndef NACHOS_FILESYS_SYNCHCONSOLE__HH
#define NACHOS_FILESYS_SYNCHCONSOLE__HH

#include "machine/console.hh"
#include "threads/lock.hh"
#include "threads/semaphore.hh"

class SynchConsole {
public:

  SynchConsole(const char *name);

  ~SynchConsole();

  

private:
  Console *console;
  Semaphore *semaphore; 
  Lock *lock;  
};

#endif