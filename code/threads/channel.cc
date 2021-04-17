#include "channel.hh"
#include "system.hh"

Channel::Channel(const char *debugName)
{
  name = debugName;
  buffer = nullptr;
  sendLock = new Lock("send lock channel");
  receiveLock = new Lock("receive lock channel");
  syncSem1 = new Semaphore("channel sem 1", 0);
  syncSem2 = new Semaphore("channel sem 2", 0);
};

Channel::~Channel()
{
  delete sendLock;
  delete receiveLock;
  delete syncSem1;
  delete syncSem2;
};

void
Channel::Send(int message)
{
  sendLock->Acquire();
  syncSem1->P();
  ASSERT(buffer != nullptr);
  *buffer = message;
  syncSem2->V();
  sendLock->Release();
};

void
Channel::Receive(int *message)
{
  receiveLock->Acquire();
  ASSERT(buffer == nullptr);
  buffer = message;
  syncSem1->V();
  syncSem2->P();
  buffer = nullptr;
  receiveLock->Release();
};