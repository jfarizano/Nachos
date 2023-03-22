#ifndef NACHOS_FILESYS_SYNCHFILE__HH
#define NACHOS_FILESYS_SYNCHFILE__HH

class Condition;
class Lock;
class Thread;

// Based on: https://en.wikipedia.org/wiki/Readers%E2%80%93writer_lock

class SynchFile {
  public:
  
    SynchFile();

    ~SynchFile();

    void BeginRead(Thread *threadAsking);

    void EndRead();

    void BeginWrite(Thread *threadAsking);

    void EndWrite();

  private:
    Lock *lock;
    Condition *cond;
    unsigned numWritersWaiting, numReadersActive;
    Thread *threadWriting;
};

#endif