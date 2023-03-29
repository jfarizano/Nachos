/// Copyright (c) 1992-1993 The Regents of the University of California.
///               2016-2021 Docentes de la Universidad Nacional de Rosario.
/// All rights reserved.  See `copyright.h` for copyright notice and
/// limitation of liability and disclaimer of warranty provisions.


#include "executable.hh"
#include "machine/endianness.hh"


/// Do little endian to big endian conversion on the bytes in the object file
/// header, in case the file was generated on a little endian machine, and we
/// are re now running on a big endian machine.
static void
SwapHeader(noffHeader *h)
{
    ASSERT(h != nullptr);

    h->noffMagic              = WordToHost(h->noffMagic);
    h->code.size              = WordToHost(h->code.size);
    h->code.virtualAddr       = WordToHost(h->code.virtualAddr);
    h->code.inFileAddr        = WordToHost(h->code.inFileAddr);
    h->initData.size          = WordToHost(h->initData.size);
    h->initData.virtualAddr   = WordToHost(h->initData.virtualAddr);
    h->initData.inFileAddr    = WordToHost(h->initData.inFileAddr);
    h->uninitData.size        = WordToHost(h->uninitData.size);
    h->uninitData.virtualAddr = WordToHost(h->uninitData.virtualAddr);
    h->uninitData.inFileAddr  = WordToHost(h->uninitData.inFileAddr);
}

Executable::Executable(OpenFile *new_file)
{
    ASSERT(new_file != nullptr);

    file = new_file;
    file->ReadAt((char *) &header, sizeof header, 0);
}

bool
Executable::CheckMagic()
{
    if (header.noffMagic != NOFF_MAGIC &&
          WordToHost(header.noffMagic) == NOFF_MAGIC) {
        SwapHeader(&header);
    }
    return header.noffMagic == NOFF_MAGIC;
}

uint32_t
Executable::GetSize() const
{
    return header.code.size + header.initData.size + header.uninitData.size;
}

uint32_t
Executable::GetCodeSize() const
{
    return header.code.size;
}

uint32_t
Executable::GetInitDataSize() const
{
    return header.initData.size;
}

uint32_t
Executable::GetUninitDataSize() const
{
    return header.uninitData.size;
}

uint32_t
Executable::GetCodeAddr() const
{
    return header.code.virtualAddr;
}

uint32_t
Executable::GetInitDataAddr() const
{
    return header.initData.virtualAddr;
}

int
Executable::ReadCodeBlock(char *dest, uint32_t size, uint32_t offset)
{
    ASSERT(dest != nullptr);
    ASSERT(size != 0);
    ASSERT(offset < header.code.size);

    return file->ReadAt(dest, size, header.code.inFileAddr + offset);
}

int
Executable::ReadDataBlock(char *dest, uint32_t size, uint32_t offset)
{
    ASSERT(dest != nullptr);
    ASSERT(size != 0);
    ASSERT(offset < header.initData.size);

    return file->ReadAt(dest, size, header.initData.inFileAddr + offset);
}
