/// Routines to manage address spaces (memory for executing user programs).
///
/// Copyright (c) 1992-1993 The Regents of the University of California.
///               2016-2021 Docentes de la Universidad Nacional de Rosario.
/// All rights reserved.  See `copyright.h` for copyright notice and
/// limitation of liability and disclaimer of warranty provisions.


#include <stdlib.h>
#include "address_space.hh"
#include "executable.hh"
#include "threads/system.hh"

#ifdef USE_SWAP
#include "vmem/coremap.hh"
#include "filesys/directory_entry.hh"
#endif

#include <string.h>
#include <cstdio>


/// First, set up the translation from program memory to physical memory.
/// For now, this is really simple (1:1), since we are only uniprogramming,
/// and we have a single unsegmented page table.
AddressSpace::AddressSpace(OpenFile *executable_file, int pid)
{
    ASSERT(executable_file != nullptr);

    exec = executable_file;

    Executable exe (executable_file);
    ASSERT(exe.CheckMagic());

    codeAddr = exe.GetCodeAddr();
    initDataAddr = exe.GetInitDataAddr();
    codeSize = exe.GetCodeSize();
    initDataSize = exe.GetInitDataSize();

    // How big is address space?
    unsigned size = exe.GetSize() + USER_STACK_SIZE;
    // We need to increase the size to leave room for the stack.
    numPages = DivRoundUp(size, PAGE_SIZE);
    size = numPages * PAGE_SIZE;

    DEBUG('a', "Initializing address space, num pages %u, size %u\n",
          numPages, size);

    #ifdef USE_SWAP
    nameSwap = new char[FILE_NAME_MAX_LEN];
    DEBUG('a', "Creating swap file\n");
    snprintf(nameSwap, FILE_NAME_MAX_LEN, "SWAP.%u", pid);
    ASSERT(fileSystem->Create(nameSwap, size));
    ASSERT(swap = fileSystem->Open(nameSwap));
    DEBUG('a', "Swap file created\n");
    #endif
    inSwap = new Bitmap(numPages);

    #ifndef DEMAND_LOADING
    // Con demand loading no cargamos datos en este punto
    char *mainMemory = machine->GetMMU()->mainMemory;
    #ifndef USE_SWAP
    ASSERT(numPages <= usedPages->CountClear());
    #endif
    #endif

    DEBUG('a', "Creating page table\n");
    // First, set up the translation.
    pageTable = new TranslationEntry[numPages];
    for (unsigned i = 0; i < numPages; i++) {
        pageTable[i].use          = false;
        pageTable[i].dirty        = false;
        pageTable[i].readOnly     = false;
        pageTable[i].virtualPage  = i;
        #ifndef DEMAND_LOADING
        #ifndef USE_SWAP
        int frame = usedPages->Find();
        #else
        int frame = usedPages->Find(this, i);
        #endif
        pageTable[i].physicalPage = frame;
        pageTable[i].valid        = true;
        // If the code segment was entirely on a separate page, we could
        // set its pages to be read-only.
        memset(mainMemory + frame * PAGE_SIZE, 0, PAGE_SIZE);
        #else
        pageTable[i].physicalPage = NUM_PHYS_PAGES + 1;
        pageTable[i].valid        = false;
        #endif
    }
    DEBUG('a', "Page table created\n");

    #ifndef DEMAND_LOADING
    // Then, copy in the code and data segments into memory.
    if (codeSize > 0) {
        uint32_t virtualAddr = codeAddr;
        DEBUG('a', "Initializing code segment, at 0x%X, size %u\n",
              virtualAddr, codeSize);
        for (uint32_t i = 0; i < codeSize; i++) {
          // Calculamos el marco físico donde se encuentra
          uint32_t frame = pageTable[DivRoundDown(virtualAddr + i, PAGE_SIZE)].physicalPage;
          // Donde dentro de ese marco nos encontramos
          uint32_t offset = (virtualAddr + i) % PAGE_SIZE;
          // Y lo traducimos a la dirección física
          uint32_t physicalAddr = frame * PAGE_SIZE + offset;
          exe.ReadCodeBlock(&mainMemory[physicalAddr], 1, i);
        }
    }
    if (initDataSize > 0) {
        uint32_t virtualAddr = initDataAddr;
        DEBUG('a', "Initializing data segment, at 0x%X, size %u\n",
              virtualAddr, initDataSize);
        for (uint32_t i = 0; i < initDataSize; i++) {
          // Calculamos el marco físico donde se encuentra
          uint32_t frame = pageTable[DivRoundDown(virtualAddr + i, PAGE_SIZE)].physicalPage;
          // Donde dentro de ese marco nos encontramos
          uint32_t offset = (virtualAddr + i) % PAGE_SIZE;
          // Y lo traducimos a la dirección física
          uint32_t physicalAddr = frame * PAGE_SIZE + offset;
          exe.ReadDataBlock(&mainMemory[physicalAddr], 1, i);
        }
    }
    #endif

    DEBUG('a', "Address space initializated\n");
}

/// Deallocate an address space.
///
/// Nothing for now!
AddressSpace::~AddressSpace()
{
    for (unsigned i = 0; i < numPages; i++) {
      if (pageTable[i].valid) {
        usedPages->Clear(pageTable[i].physicalPage);
      }
    }

    delete exec;
    delete [] pageTable;

    #ifdef USE_SWAP
    delete [] nameSwap;
    delete inSwap;
    #endif
}

/// Set the initial values for the user-level register set.
///
/// We write these directly into the “machine” registers, so that we can
/// immediately jump to user code.  Note that these will be saved/restored
/// into the `currentThread->userRegisters` when this thread is context
/// switched out.
void
AddressSpace::InitRegisters()
{
    for (unsigned i = 0; i < NUM_TOTAL_REGS; i++) {
        machine->WriteRegister(i, 0);
    }

    // Initial program counter -- must be location of `Start`.
    machine->WriteRegister(PC_REG, 0);

    // Need to also tell MIPS where next instruction is, because of branch
    // delay possibility.
    machine->WriteRegister(NEXT_PC_REG, 4);

    // Set the stack register to the end of the address space, where we
    // allocated the stack; but subtract off a bit, to make sure we do not
    // accidentally reference off the end!
    machine->WriteRegister(STACK_REG, numPages * PAGE_SIZE - 16);
    DEBUG('a', "Initializing stack register to %u\n",
          numPages * PAGE_SIZE - 16);
}

/// On a context switch, save any machine state, specific to this address
/// space, that needs saving.
///
/// For now, nothing!
void
AddressSpace::SaveState()
{
  #ifdef USE_TLB
  unsigned vpn;
  TranslationEntry *tlb = machine->GetMMU()->tlb;

  for (unsigned i = 0; i < TLB_SIZE; i++) {
    if (tlb[i].valid) {
      vpn = tlb[i].virtualPage;
      pageTable[vpn].use = tlb[i].use;
      pageTable[vpn].dirty = tlb[i].dirty;
    }
  }
  #endif
}

/// On a context switch, restore the machine state so that this address space
/// can run.
///
void
AddressSpace::RestoreState()
{
    #ifdef USE_TLB
      DEBUG('a', "TLB has been invalidated\n");
      for (unsigned i = 0; i < TLB_SIZE; i++) {
          machine->GetMMU()->tlb[i].valid = false;
      }
    #else
      machine->GetMMU()->pageTable     = pageTable;
      machine->GetMMU()->pageTableSize = numPages;
    #endif
}

TranslationEntry*
AddressSpace::GetTranslationEntry(unsigned vpn) {
    ASSERT(vpn >= 0 && vpn < numPages);

    return &pageTable[vpn];
}

#ifdef VMEM
void
AddressSpace::LoadPage(unsigned vpn)
{  
  ASSERT(!pageTable[vpn].valid);

  #if defined(DEMAND_LOADING) || defined(USE_SWAP)
  unsigned virtualAddr = vpn * PAGE_SIZE;
  #endif

  #ifndef USE_SWAP
  int frame = usedPages->Find();
  #else
  int frame = usedPages->Find(this, vpn);
  if (frame == - 1) {
    frame = PickVictim();
    HandleVictim(frame);
    usedPages->Mark(frame, this, vpn);
  }
  #endif
  ASSERT(frame != -1);

  char *mainMemory = machine->GetMMU()->mainMemory;
  uint32_t physicalAddr = frame * PAGE_SIZE;
  memset(mainMemory + physicalAddr, 0, PAGE_SIZE);

  #ifdef DEMAND_LOADING
  // No se cargó nunca por demand loading y no está en swap
  if (!inSwap->Test(vpn)) {
    Executable exe (exec);
    ASSERT(exe.CheckMagic());

    DEBUG('e', "Page demand loaded\n");
    unsigned bytesRead = 0;
    // We are in code segment
    if (codeSize > 0 && virtualAddr < codeSize) {
      unsigned codeBlockSize = codeSize - virtualAddr;
      codeBlockSize = codeBlockSize < PAGE_SIZE ? codeBlockSize : PAGE_SIZE; 
      exe.ReadCodeBlock(&mainMemory[physicalAddr], codeBlockSize, virtualAddr);
      bytesRead += codeBlockSize;
      DEBUG('e', "Code loaded\n");
    }
    
    // We are in init data segment
    if (bytesRead < PAGE_SIZE && initDataSize > 0 && virtualAddr + bytesRead < initDataAddr + initDataSize) {
      unsigned offset = bytesRead > 0 ? 0 : virtualAddr - codeSize;
      unsigned initDataBlockSize = initDataSize - offset < PAGE_SIZE - bytesRead ? initDataSize - offset : PAGE_SIZE - bytesRead;
      exe.ReadDataBlock(&mainMemory[physicalAddr + bytesRead], initDataBlockSize, offset);
      bytesRead += initDataBlockSize;
      DEBUG('e', "Data loaded\n");
    }
    
    stats->numPagesDemandLoaded++;
    ASSERT(bytesRead <= PAGE_SIZE);
  }
  #endif

  #ifdef USE_SWAP
  // The page is in swap
  if (inSwap->Test(vpn)) {
    DEBUG('e', "Página brought to swap\n");
    ASSERT(swap->ReadAt(&mainMemory[physicalAddr], PAGE_SIZE, virtualAddr) == PAGE_SIZE);
    stats->numBroughtSwap++;
  }
  #endif

  pageTable[vpn].physicalPage = frame;
  pageTable[vpn].valid        = true;
}
#endif

#ifdef USE_SWAP
void
AddressSpace::HandleVictim(unsigned frame)
{
  int vpn = usedPages->GetVpn(frame);
  AddressSpace* space = usedPages->GetAddrSpace(frame);
  TranslationEntry *entry = space->GetTranslationEntry(vpn);
  TranslationEntry *tlb = machine->GetMMU()->tlb;

  ASSERT(vpn != -1);
  ASSERT(space != nullptr);

  DEBUG('e', "Freeing frame %u, occupied por vpn %u\n", frame, vpn);  
  
  // TODO: No es necesario si no es el space actual
  for (unsigned i = 0; i < TLB_SIZE; i++) {
    if (tlb[i].physicalPage == frame && tlb[i].valid) {
      entry->dirty = tlb[i].dirty;
      entry->use = tlb[i].use;
      tlb[i].valid = false;
      break;
    }
  }

  if (entry->dirty) {
    DEBUG('e', "Sending page to swap\n");
    char *mainMemory = machine->GetMMU()->mainMemory;
    unsigned virtualAddr = vpn * PAGE_SIZE;
    uint32_t physicalAddr = frame * PAGE_SIZE;
    ASSERT(space->swap->WriteAt(&mainMemory[physicalAddr], PAGE_SIZE, virtualAddr) == PAGE_SIZE);
    space->inSwap->Mark(vpn);
    stats->numSentSwap++;
    DEBUG('e', "Page sent to swap\n");
  }

  entry->dirty = false;
  entry->valid = false;

  DEBUG('e', "Frame %u freed\n", frame); 
}

// Second Chance Improved algorithm for picking a victim
unsigned
ClockPolicy() 
{
  static unsigned victim = 0;
  unsigned temp;
  int vpn;
  AddressSpace* space;
  TranslationEntry *entry;

  currentThread->space->SaveState();
  
  // The algorithm makes at most 4 rounds
  for (unsigned round = 1; round <= 4; round++) {
    for (unsigned i = 0; i < NUM_PHYS_PAGES; i++) {
      entry = nullptr;

      // If it's not in the TLB, we look for it in some address space
      if (entry == nullptr) {
        space = usedPages->GetAddrSpace(victim);
        if (space != nullptr) {
          vpn = usedPages->GetVpn(victim);
          entry = space->GetTranslationEntry(vpn);
        }
      }
      // We found it! :D
      if (entry != nullptr) {
        if (round == 1 || round == 3) {
          // We look for (0, 0)
          if(!entry->use && !entry->dirty) {
            temp = victim;
            victim++;
            victim %= NUM_PHYS_PAGES;
            return temp;
          }
        } else if (round == 2 || round == 4) {
          // We look for (0, 1)
          if(!entry->use && entry->dirty) {
            temp = victim;
            victim++;
            victim %= NUM_PHYS_PAGES;
            return temp;
          } else {
            // If we are in the second round we have to set the use bit to false
            if (round == 2) {
              entry->use = false;
            }
          }
        }
      // It's free real estate
      } else {
        temp = victim;
        victim++;
        victim %= NUM_PHYS_PAGES;
        return temp;
      }
      victim++;
      victim %= NUM_PHYS_PAGES;
    }
  }

  // The function will always return in one of the rounds above, this is just
  // for the compiler
  return victim;
}

unsigned
AddressSpace::PickVictim()
{
  #ifdef PRPOLICY_FIFO
    static int victim = -1;
    victim++;
    victim %= NUM_PHYS_PAGES;
    return victim;
  #elif PRPOLICY_CLOCK
    return ClockPolicy();
  #else 
    // If no policy is selected we use a random number as policy
    return random() % NUM_PHYS_PAGES;
  #endif

}
#endif