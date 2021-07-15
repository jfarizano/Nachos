#include "coremap.hh"

Coremap::Coremap(unsigned nitems)
{
  framesMap = new Bitmap(nitems);
  addrSpaces = new AddressSpace*[nitems];
  vpns = new unsigned[nitems];
  size = nitems; 
}

Coremap::~Coremap()
{
  delete framesMap;
  delete [] addrSpaces;
  delete [] vpns;
}

void
Coremap::Mark(unsigned which, AddressSpace *space, unsigned vpn) 
{
  framesMap->Mark(which);
  addrSpaces[which] = space;
  vpns[which] = vpn;
}

void
Coremap::Clear(unsigned which) 
{
  framesMap->Clear(which);
  addrSpaces[which] = nullptr;
}

bool
Coremap::Test(unsigned which) const
{
  return framesMap->Test(which) && addrSpaces[which] != nullptr; 
}

int
Coremap::Find(AddressSpace *space, unsigned vpn)
{
  int which = framesMap->Find();
  if (which != -1) {
    addrSpaces[which] = space;
    vpns[which] = vpn;
  }
  #ifdef USE_SWAP
  else {

  }
  #endif
  
  return which;
}

unsigned
Coremap::CountClear() const
{
  return framesMap->CountClear();
}

#ifdef USE_SWAP
int PickVictim() {

  #ifdef PRPOLICY_FIFO
    static int victim = 0;
    victim++;
    victim %= NUM_PHYS_PAGES;
    return victim

  // If no policy is selected we use a random number as policy
  #else 
    return random() % NUM_PHYS_PAGES;
  #endif

}
#endif