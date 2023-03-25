#ifndef NACHOS_FILESYS_SYNCHDIRECTORY__HH
#define NACHOS_FILESYS_SYNCHDIRECTORY__HH

#include "threads/lock.hh"
#include "directory.hh"

/// Wrapper class for Directory with syncronized access.
class SynchDirectory {
public:

    SynchDirectory(unsigned size, Lock *dirLock);
    
    ~SynchDirectory();

    /// Initialize directory contents from disk.
    void FetchFrom(OpenFile *file);

    /// Write modifications to directory contents back to disk.
    void WriteBack(OpenFile *file);

    /// The lock is acquired manually.
    void Request();

    /// The lock was acquired but no change was made, the lock is released.
    void Flush();
    
    /// Find the sector number of the `FileHeader` for file: `name`.
    int Find(const char *name);

    /// Add a file name into the directory.
    bool Add(const char *name, int newSector);

    /// Remove a file from the directory.
    bool Remove(const char *name);

    /// Print the names of all the files in the directory.
    void List() const;

    /// Verbose print of the contents of the directory -- all the file names
    /// and their contents.
    void Print() const;

    /// Get the raw directory structure.
    ///
    /// NOTE: this should only be used by routines that operating on the file
    /// system at a low level.
    const RawDirectory *GetRaw() const;


private:
    Directory  *directory;
    Lock * directoryLock;
};

#endif