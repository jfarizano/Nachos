#ifndef NACHOS_LIB_COREMAP__HH
#define NACHOS_LIB_COREMAP__HH


#include "lib/bitmap.hh"
#include "userprog/address_space.hh"

class Coremap {
public:

    /// Initialize a coremap with `nitems` bits; all bits are cleared.
    ///
    /// * `nitems` is the number of items in the bitmap.
    Coremap(unsigned nitems);

    /// Uninitialize a coremap.
    ~Coremap();

    /// Set the “nth” bit.
    void Mark(unsigned which, AddressSpace *space, unsigned vpn);

    /// Clear the “nth” bit.
    void Clear(unsigned which);

    /// Is the “nth” bit set?
    bool Test(unsigned which) const;

    /// Given an address space and a virtual page, finds a physical frame.
    /// If a frame can't be found it returns -1.
    int Find(AddressSpace *space, unsigned vpn);

    /// Return the number of clear bits.
    unsigned CountClear() const;

    unsigned size;

    AddressSpace *GetAddrSpace(unsigned frame);

    int GetVpn(unsigned frame);
    
private:

   Bitmap *framesMap;
   AddressSpace **addrSpaces;
   unsigned *vpns;
};


#endif