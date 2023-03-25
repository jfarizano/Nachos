#include "synch_directory.hh"

SynchDirectory::SynchDirectory(unsigned size, Lock *dirLock) 
{
    directory = new Directory(size);
    directoryLock = dirLock;
}

SynchDirectory::~SynchDirectory()
{
    delete directory;
}

/// Read the contents of the directory from disk.
///
/// * `file` is file containing the directory contents.
void
SynchDirectory::FetchFrom(OpenFile *file)
{
    DEBUG('f', "Locking directory\n");
    directoryLock->Acquire();
    DEBUG('f', "Directory locked\n");
    directory->FetchFrom(file);
}

/// Write any modifications to the directory back to disk.
///
/// * `file` is a file to contain the new directory contents.
void
SynchDirectory::WriteBack(OpenFile *file)
{
    directory->WriteBack(file);
    directoryLock->Release();
    DEBUG('f', "Directory released\n");
}

// It's possible to do a WriteBack without doing a FetchFrom first, so we 
// acquire the lock manually
void
SynchDirectory::Request() {
    DEBUG('f', "Locking directory\n");
    directoryLock->Acquire();
    DEBUG('f', "Directory locked\n");
}

/// The lock was acquired but no change was made, the lock is released.
/// After this, there shouldn't be a WriteBack and the Directory will be deleted
void
SynchDirectory::Flush() {
    directoryLock->Release();
    DEBUG('f', "Directory released\n");
}

/// Look up file name in directory, and return the disk sector number where
/// the file's header is stored.  Return -1 if the name is not in the
/// directory.
///
/// * `name` is the file name to look up.
int
SynchDirectory::Find(const char *name)
{
    return directory->Find(name);
}

/// Add a file into the directory.  Return true if successful; return false
/// if the file name is already in the directory, or if the directory is
/// completely full, and has no more space for additional file names.
///
/// * `name` is the name of the file being added.
/// * `newSector` is the disk sector containing the added file's header.
bool
SynchDirectory::Add(const char *name, int newSector)
{
    return directory->Add(name, newSector);
}

/// Remove a file name from the directory.   Return true if successful;
/// return false if the file is not in the directory.
///
/// * `name` is the file name to be removed.
bool
SynchDirectory::Remove(const char *name)
{
    return directory->Remove(name);
}

/// List all the file names in the directory.
void
SynchDirectory::List() const
{
    directory->List();
}

/// List all the file names in the directory, their `FileHeader` locations,
/// and the contents of each file.  For debugging.
void
SynchDirectory::Print() const
{
    directory->Print();
}

const RawDirectory *
SynchDirectory::GetRaw() const
{
    return directory->GetRaw();
}