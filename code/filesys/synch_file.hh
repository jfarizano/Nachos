#ifndef NACHOS_FILESYS_SYNCHFILE__HH
#define NACHOS_FILESYS_SYNCHFILE__HH

#include "threads/condition.hh"

class SynchFile {
  public:
    SynchFile();
    ~SynchFile();
    void BeginRead();
    void EndRead();
    void BeginWrite();
    void EndWrite();
  private:
    Lock *lock;
    Condition *cond;
    unsigned numWritersWaiting, numReadersActive;
    bool writing;
};

#endif