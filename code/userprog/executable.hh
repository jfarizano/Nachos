/// In order to run a user program, you must:
///
/// 1. Link with the `-N -T 0` option.
/// 2. Run `coff2noff` to convert the object file to Nachos format (Nachos
///    object code format is essentially just a simpler version of the UNIX
///    executable object code format).
/// 3. Load the NOFF file into the Nachos file system (if you have not
///    implemented the file system yet, you do not need to do this last
///    step).
///
/// Copyright (c) 2019-2021 Docentes de la Universidad Nacional de Rosario.
/// All rights reserved.  See `copyright.h` for copyright notice and
/// limitation of liability and disclaimer of warranty provisions.

#ifndef NACHOS_USERPROG_EXECUTABLE__HH
#define NACHOS_USERPROG_EXECUTABLE__HH


#include "bin/noff.h"
#include "filesys/open_file.hh"


/// Assumes that the object code file is in NOFF format.
class Executable {
public:
    Executable(OpenFile *new_file);

    /// Check if the executable is valid and fix endianness if necessary.
    ///
    /// Check if the executable conforms to the NOFF file format by checking
    /// the magic number at the beginning of the header.  Also detect if it
    /// is reversed (due to mismatching endianness) and in that case swap the
    /// entire header.
    bool CheckMagic();

    uint32_t GetSize() const;
    uint32_t GetCodeSize() const;
    uint32_t GetInitDataSize() const;
    uint32_t GetUninitDataSize() const;

    /// The following methods provide addresses where each segment is meant
    /// to start.  These addresses refer to main memory, not to the file.
    /// Keep in mind that when a virtual addressing mechanism is used, these
    /// addresses are to be considered virtual, not physical.

    uint32_t GetCodeAddr() const;
    uint32_t GetInitDataAddr() const;
    uint32_t GetUninitDataAddr() const;

    /// The following methods read a block from a given program segment into
    /// memory.  Reads are possible only from the code and the initialized
    /// data segments, because these are the ones that actually encode their
    /// contents in the file.  The uninitialized data segment is composed of
    /// all zeroes and these are not encoded in the file.
    ///
    /// Parameters:
    /// * `dest` is the start address of a memory array.
    /// * `size` is the amount of bytes to read.
    /// * `offset` is the relative position inside the respective segment
    ///   where the start of the block is to be found.

    int ReadCodeBlock(char *dest, uint32_t size, uint32_t offset);
    int ReadDataBlock(char *dest, uint32_t size, uint32_t offset);

private:
    OpenFile *file;
    noffHeader header;
};


#endif
