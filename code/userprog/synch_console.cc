#include "synch_console.hh"


/// Dummy functions because C++ is weird about pointers to member functions.
static void
ReadAvailDummy(void *arg)
{
    ASSERT(arg != nullptr);
    SynchConsole *console = (SynchConsole *) arg;
    console->ReadAvail();
}

static void
WriteDoneDummy(void *arg)
{
    ASSERT(arg != nullptr);
    SynchConsole *console = (SynchConsole *) arg;
    console->WriteDone();
}

SynchConsole::SynchConsole(const char *name)
{
    console = new Console(nullptr, nullptr, ReadAvailDummy, WriteDoneDummy, this);
    readAvail = new Semaphore("Read avail synch console", 0);
    writeDone = new Semaphore("Write done synch console", 0);
    readLock = new Lock("Reading synch console lock");
    writeLock = new Lock("Writing synch console lock");
}

SynchConsole::~SynchConsole()
{
    delete console;
    delete readAvail;
    delete writeDone;
    delete readLock;
    delete writeLock;
}

void
SynchConsole::ReadChar(char *c)
{
    readLock->Acquire();
    readAvail->P();
    *c = console->GetChar();
    readLock->Release();
}

void
SynchConsole::WriteChar(const char *c)
{
    writeLock->Acquire();
    console->PutChar(*c);
    writeDone->P();
    writeLock->Release();
}

void
SynchConsole::ReadBuffer(char *buffer, unsigned size)
{
    readLock->Acquire();
    for (unsigned i = 0; i < size; i++) 
    {
      readAvail->P();
      buffer[i] = console->GetChar();
    }
    readLock->Release();
}

void
SynchConsole::WriteBuffer(char *buffer, unsigned size) 
{
    writeLock->Acquire();
    for (unsigned i = 0; i < size; i++)
    {
        console->PutChar(buffer[i]);
        writeDone->P();
    }
    writeLock->Release();
}

void
SynchConsole::ReadAvail()
{
  readAvail->V();
}

void
SynchConsole::WriteDone()
{
  writeDone->V();
}