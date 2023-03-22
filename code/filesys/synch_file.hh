#ifndef NACHOS_FILESYS_SYNCHFILE__HH
#define NACHOS_FILESYS_SYNCHFILE__HH

class Condition;
class Lock;

// Based on: https://en.wikipedia.org/wiki/Readers%E2%80%93writer_lock

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