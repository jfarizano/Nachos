#include "synch_file.hh"
#include "threads/condition.hh"

SynchFile::SynchFile()
{
  lock = new Lock("file lock");
  cond = new Condition("file cond", lock);
  numWritersWaiting = 0;
  numReadersActive = 0;
  writing = false;
}

SynchFile::~SynchFile()
{
  delete lock;
  delete cond;
}

void
SynchFile::BeginRead()
{
  lock->Acquire();
  while (numWritersWaiting > 0 || writing) {
    cond->Wait();
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
SynchFile::BeginWrite()
{
  lock->Acquire();
  numWritersWaiting++;
  while (numReadersActive > 0 || writing) {
    cond->Wait();
  }
  numWritersWaiting--;
  writing = true;
  lock->Release();
}

void
SynchFile::EndWrite()
{
  lock->Acquire();
  writing = false;
  cond->Broadcast();
  lock->Release();
}