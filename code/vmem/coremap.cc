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
  
  return which;
}

unsigned
Coremap::CountClear() const
{
  return framesMap->CountClear();
}

AddressSpace*
Coremap::GetAddrSpace(unsigned frame)
{
  if (Test(frame)){
    return addrSpaces[frame];
  } else {
    return nullptr;
  }
}

int
Coremap::GetVpn(unsigned frame)
{
  if (Test(frame)){
    return vpns[frame];
  } else {
    return -1;
  }
}