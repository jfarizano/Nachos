/// System-dependent interface.
///
/// Nachos uses the routines defined here, rather than directly calling the
/// UNIX library functions, to simplify porting between versions of UNIX, and
/// even to other systems, such as MSDOS and the Macintosh.
///
/// Copyright (c) 1992-1993 The Regents of the University of California.
///               2016-2021 Docentes de la Universidad Nacional de Rosario.
/// All rights reserved.  See `copyright.h` for copyright notice and
/// limitation of liability and disclaimer of warranty provisions.

#ifndef NACHOS_MACHINE_SYSDEP__HH
#define NACHOS_MACHINE_SYSDEP__HH


#include <stddef.h>


namespace SystemDep {
    /// Check file to see if there are any characters to be read.
    ///
    /// If no characters in the file, return without waiting.
    bool PollFile(int fd);

    /// File operations: `open`/`read`/`write`/`lseek`/`close`, and check for
    /// error.
    ///
    /// For simulating the disk and the console devices.

    int OpenForWrite(const char *name);

    int OpenForReadWrite(const char *name, bool crashOnError);

    void Read(int fd, char *buffer, size_t nBytes);

    int ReadPartial(int fd, char *buffer, size_t nBytes);

    void WriteFile(int fd, const char *buffer, size_t nBytes);

    void Lseek(int fd, int offset, int whence);

    int Tell(int fd);

    void Close(int fd);

    bool Unlink(const char *name);

    /// Interprocess communication operations, for simulating the network.

    int OpenSocket();

    void CloseSocket(int sockID);

    void AssignNameToSocket(const char *socketName, int sockID);

    void DeAssignNameToSocket(const char *socketName);

    bool PollSocket(int sockID);

    void ReadFromSocket(int sockID, char *buffer, size_t packetSize);

    void SendToSocket(int sockID, const char *buffer,
                      size_t packetSize, const char *toName);

    /// Process control: `sleep`.

    void Delay(unsigned seconds);

    /// Initialize system so that `cleanUp` routine is called when user hits
    /// Ctrl-C.
    void CallOnUserAbort(VoidNoArgFunctionPtr cleanUp);

    /// Initialize the pseudo random number generator.
    void RandomInit(unsigned seed);

    int Random();

    /// Allocate, de-allocate an array, such that de-referencing just beyond
    /// either end of the array will cause an error.

    char *AllocBoundedArray(unsigned size);

    void DeallocBoundedArray(const char *p, unsigned size);
};


#endif
