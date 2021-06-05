/// Routines to manage address spaces (memory for executing user programs).
///
/// Copyright (c) 1992-1993 The Regents of the University of California.
///               2016-2021 Docentes de la Universidad Nacional de Rosario.
/// All rights reserved.  See `copyright.h` for copyright notice and
/// limitation of liability and disclaimer of warranty provisions.


#include "address_space.hh"
#include "executable.hh"
#include "threads/system.hh"
#include "lib/bitmap.hh"

#include <string.h>


/// First, set up the translation from program memory to physical memory.
/// For now, this is really simple (1:1), since we are only uniprogramming,
/// and we have a single unsegmented page table.
AddressSpace::AddressSpace(OpenFile *executable_file)
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

    ASSERT(numPages <= usedPages->CountClear());
      // Check we are not trying to run anything too big -- at least until we
      // have virtual memory.

    DEBUG('a', "Initializing address space, num pages %u, size %u\n",
          numPages, size);

    #ifndef DEMAND_LOADING
    char *mainMemory = machine->GetMMU()->mainMemory;
    #endif

    // First, set up the translation.
    pageTable = new TranslationEntry[numPages];
    for (unsigned i = 0; i < numPages; i++) {
        pageTable[i].use          = false;
        pageTable[i].dirty        = false;
        pageTable[i].readOnly     = false;
        #ifndef DEMAND_LOADING
        int frame = usedPages->Find();
        pageTable[i].virtualPage  = i;
        pageTable[i].physicalPage = frame;
        pageTable[i].valid        = true;
          // If the code segment was entirely on a separate page, we could
          // set its pages to be read-only.
        memset(mainMemory + frame * PAGE_SIZE, 0, PAGE_SIZE);
        #else
        pageTable[i].virtualPage  = numPages + 1;
        pageTable[i].physicalPage = 0;
        pageTable[i].valid        = false;
        #endif
    }

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
}

/// Deallocate an address space.
///
/// Nothing for now!
AddressSpace::~AddressSpace()
{
    for (unsigned i = 0; i < numPages; i++) {
        usedPages->Clear(pageTable[i].physicalPage);
    }

    delete exec;
    delete [] pageTable;
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
{}

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

TranslationEntry
AddressSpace::GetTranslationEntry(unsigned vpn) {
    ASSERT(vpn >= 0 && vpn < numPages);

    return pageTable[vpn];
}

#ifdef DEMAND_LOADING
int
AddressSpace::LoadPage(unsigned vpn)
{

  Executable exe (exec);
  ASSERT(exe.CheckMagic());
  ASSERT(pageTable[vpn].virtualPage == currentThread->space->numPages + 1);

  unsigned virtualAddr = vpn * PAGE_SIZE;
  unsigned bytesRead = 0;

  int frame = usedPages->Find();
  pageTable[vpn].physicalPage = frame;
  pageTable[vpn].virtualPage  = vpn;
  pageTable[vpn].valid        = true;

  char *mainMemory = machine->GetMMU()->mainMemory;
  memset(mainMemory + frame * PAGE_SIZE, 0, PAGE_SIZE);
  uint32_t physicalAddr = frame * PAGE_SIZE;

  // We are in code segment
  if (virtualAddr < codeSize) {
    unsigned codeBlockSize = codeSize - virtualAddr;
    exe.ReadCodeBlock(&mainMemory[physicalAddr], codeBlockSize, virtualAddr);
    bytesRead += codeBlockSize;
  }

  // We are in init data segment
  if (virtualAddr + bytesRead < initDataAddr + initDataSize) {
    unsigned initDataBlockSize = (initDataAddr + initDataSize) - (virtualAddr + bytesRead);
    unsigned offset = bytesRead > 0 ? 0 : virtualAddr - codeSize;
    exe.ReadDataBlock(&mainMemory[physicalAddr + bytesRead], initDataBlockSize, offset);
    bytesRead += initDataBlockSize;
  }

  return frame;
}
#endif