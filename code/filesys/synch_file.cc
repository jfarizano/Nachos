#include "synch_file.hh"
#include "threads/condition.hh"

SynchFile::SynchFile()
{
  lock = new Lock("file lock");
  cond = new Condition("file cond", lock);
  numWritersWaiting = 0;
  numReadersActive = 0;
  threadWriting = nullptr;
}

SynchFile::~SynchFile()
{
  delete lock;
  delete cond;
}

void
SynchFile::BeginRead(Thread *threadAsking)
{
  lock->Acquire();
  
  if (threadAsking != threadWriting) {
    while (numWritersWaiting > 0 || threadWriting != nullptr) {
      cond->Wait();
    }
  }
  numReadersActive++;
  lock->Release();
}

void
SynchFile::EndRead()
{
  lock->Acquire();
  numReadersActive--;
  if (numReadersActive == 0) {
    cond->Broadcast();
  }
  lock->Release();
}

void
SynchFile::BeginWrite(Thread *threadAsking)
{
  lock->Acquire();
  numWritersWaiting++;
  while (numReadersActive > 0 || threadWriting != nullptr) {
    cond->Wait();
  }
  numWritersWaiting--;
  threadWriting = threadAsking;
  lock->Release();
}

void
SynchFile::EndWrite()
{
  lock->Acquire();
  cond->Broadcast();
  threadWriting = nullptr;
  lock->Release();
}