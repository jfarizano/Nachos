#include "synch_bitmap.hh"
// #include "open_file.hh"
// #include "lib/bitmap.hh"
// #include "threads/lock.hh"

/// Initialize a bitmap with `nitems` bits, so that every bit is clear.  It
/// can be added somewhere on a list.
///
/// * `nitems` is the number of bits in the bitmap.
SynchBitmap::SynchBitmap(unsigned nitems, Lock *bitLock)
{
    bitmap = new Bitmap(nitems);
    bitmapLock = bitLock;
}

/// De-allocate a bitmap.
SynchBitmap::~SynchBitmap()
{
    delete bitmap;
}

/// Set the “nth” bit in a bitmap.
///
/// * `which` is the number of the bit to be set.
void
SynchBitmap::Mark(unsigned which)
{
    bitmap->Mark(which);
}

/// Clear the “nth” bit in a bitmap.
///
/// * `which` is the number of the bit to be cleared.
void
SynchBitmap::Clear(unsigned which)
{
    bitmap->Clear(which);
}

/// Return true if the “nth” bit is set.
///
/// * `which` is the number of the bit to be tested.
bool
SynchBitmap::Test(unsigned which) const
{
    return bitmap->Test(which);
}

/// Return the number of the first bit which is clear.  As a side effect, set
/// the bit (mark it as in use).  (In other words, find and allocate a bit.)
///
/// If no bits are clear, return -1.
int
SynchBitmap::Find()
{
    return bitmap->Find();
}

/// Return the number of clear bits in the bitmap.  (In other words, how many
/// bits are unallocated?)
unsigned
SynchBitmap::CountClear() const
{
    return bitmap->CountClear();
}

/// Print the contents of the bitmap, for debugging.
///
/// Could be done in a number of ways, but we just print the indexes of all
/// the bits that are set in the bitmap.
void
SynchBitmap::Print() const
{
    bitmap->Print();
}

/// Initialize the contents of a bitmap from a Nachos file.
///
///
/// * `file` is the place to read the bitmap from.
void
SynchBitmap::FetchFrom(OpenFile *file)
{
    DEBUG('f', "Locking freemap\n");
    bitmapLock->Acquire();
    DEBUG('f', "Freemap locked\n");
    bitmap->FetchFrom(file);
}

/// Store the contents of a bitmap to a Nachos file.
///
///
/// * `file` is the place to write the bitmap to.
void
SynchBitmap::WriteBack(OpenFile *file) const
{
    bitmap->WriteBack(file);
    bitmapLock->Release();
    DEBUG('f', "Free map released\n");
}

// It's possible to do a WriteBack without doing a FetchFrom first, so we 
// acquire the lock manually
void
SynchBitmap::Request() {
    DEBUG('f', "Locking freemap\n");
    bitmapLock->Acquire();
    DEBUG('f', "Freemap locked\n");
}

/// The lock was acquired but no change was made, the lock is released.
/// After this, there shouldn't be a WriteBack and the Directory will be deleted
void
SynchBitmap::Flush()
{
  bitmapLock->Release();
  DEBUG('f', "Free map released\n");
}

Bitmap*
SynchBitmap::GetBitmap() {
    return bitmap;
}
