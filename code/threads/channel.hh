
#ifndef NACHOS_THREADS_CHANNEL__HH
#define NACHOS_THREADS_CHANNEL__HH

#include "condition.hh"

class Channel {
public:

  Channel(const char *debugName);

  ~Channel();

  const char *GetName() const;

  void Send(int message);

  void Receive(int *message);

private:

  // Debug name
  const char *name;

  // Addres given by Receive to store message from Send
  int *buffer;

  // Semaphores used to synchronize Send and Receive
  Semaphore *syncSem1, *syncSem2;

  // Locks used to make Send and Receive mutually exclusive.
  Lock *sendLock, *receiveLock;
}; 

#endif