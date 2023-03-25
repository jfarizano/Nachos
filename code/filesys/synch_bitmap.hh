#ifndef NACHOS_FILESYS_SYNCHBITMAP__HH
#define NACHOS_FILESYS_SYNCHBITMAP__HH

#include "lib/bitmap.hh"
#include "threads/lock.hh"

/// Wrapper for Bitmap class with synchronized access.
class SynchBitmap {
public:

    /// Initialize a bitmap with `nitems` bits; all bits are cleared.
    ///
    /// * `nitems` is the number of items in the bitmap.
    SynchBitmap(unsigned nitems, Lock *bitLock);

    /// Uninitialize a bitmap.
    ~SynchBitmap();

    /// Set the “nth” bit.
    void Mark(unsigned which);

    /// Clear the “nth” bit.
    void Clear(unsigned which);

    /// Is the “nth” bit set?
    bool Test(unsigned which) const;

    /// Return the index of a clear bit, and as a side effect, set the bit.
    ///
    /// If no bits are clear, return -1.
    int Find();

    /// Return the number of clear bits.
    unsigned CountClear() const;

    /// Print contents of bitmap.
    void Print() const;

    /// Fetch contents from disk.
    void FetchFrom(OpenFile *file);

    /// Write contents to disk.
    void WriteBack(OpenFile *file) const;

    /// The lock is acquired manually.
    void Request();

    /// The lock was acquired but no change was made, the lock is released.
    void Flush();

    // Get the original bitmap
    Bitmap* GetBitmap();

private:

    Bitmap *bitmap;
    Lock *bitmapLock;

};

#endif
