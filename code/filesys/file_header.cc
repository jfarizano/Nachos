/// Routines for managing the disk file header (in UNIX, this would be called
/// the i-node).
///
/// The file header is used to locate where on disk the file's data is
/// stored.  We implement this as a fixed size table of pointers -- each
/// entry in the table points to the disk sector containing that portion of
/// the file data (in other words, there are no indirect or doubly indirect
/// blocks). The table size is chosen so that the file header will be just
/// big enough to fit in one disk sector,
///
/// Unlike in a real system, we do not keep track of file permissions,
/// ownership, last modification date, etc., in the file header.
///
/// A file header can be initialized in two ways:
///
/// * for a new file, by modifying the in-memory data structure to point to
///   the newly allocated data blocks;
/// * for a file already on disk, by reading the file header from disk.
///
/// Copyright (c) 1992-1993 The Regents of the University of California.
///               2016-2021 Docentes de la Universidad Nacional de Rosario.
/// All rights reserved.  See `copyright.h` for copyright notice and
/// limitation of liability and disclaimer of warranty provisions.


#include "file_header.hh"
#include "threads/system.hh"

#include <ctype.h>
#include <stdio.h>


/// Initialize a fresh file header for a newly created file.  Allocate data
/// blocks for the file out of the map of free disk blocks.  Return false if
/// there are not enough free blocks to accomodate the new file.
///
/// * `freeMap` is the bit map of free disk sectors.
/// * `fileSize` is the bit map of free disk sectors.
bool
FileHeader::Allocate(Bitmap *freeMap, unsigned fileSize)
{
    ASSERT(freeMap != nullptr);

    if (fileSize > MAX_FILE_SIZE) {
        return false;
    }

    raw.numBytes = fileSize;

    unsigned numDataSectors = GetNumDataSectors();
    // data + indirec tables, raw file header already has a sector
    unsigned numIndirectTables = GetNumIndirectTables();
    unsigned numSectorsTotal = numDataSectors + numIndirectTables;
    
    if (freeMap->CountClear() < numSectorsTotal) {
        return false;  // Not enough space.
    }

    // Alojamos un sector para cada tabla de indirección
    for (unsigned i = 0; i < numIndirectTables; i++) {
        raw.tableSectors[i] = freeMap->Find();
    }

    // Para cada sector de datos, calculamos su tabla de indirección y dentro de
    // ella alojamos espacio para él.
    for (unsigned i = 0, indexInTable = 0; i < numDataSectors; i++) {
        unsigned table = DivRoundDown(i, NUM_DIRECT);
        indirectTables[table].dataSectors[indexInTable] = freeMap->Find();
        indexInTable++;
        indexInTable %= NUM_DIRECT;
    }
    
    return true;
}

/// De-allocate all the space allocated for data blocks for this file.
///
/// * `freeMap` is the bit map of free disk sectors.
void
FileHeader::Deallocate(Bitmap *freeMap)
{
    ASSERT(freeMap != nullptr);

    unsigned numDataSectors = GetNumDataSectors();
    unsigned numIndirectTables = GetNumIndirectTables();

    // Liberamos todos los sectores ocupados
    for (unsigned i = 0, indexInTable = 0; i < numDataSectors; i++) {
        unsigned table = DivRoundDown(i, NUM_DIRECT);
        ASSERT(freeMap->Test(indirectTables[table].dataSectors[indexInTable]));  // ought to be marked!
        freeMap->Clear(indirectTables[table].dataSectors[indexInTable]);
        indexInTable++;
        indexInTable %= NUM_DIRECT;
    }

    // Liberamos las tablas de indirección
    for (unsigned i = 0; i < numIndirectTables; i++) {
        ASSERT(freeMap->Test(raw.tableSectors[i]));
        freeMap->Clear(raw.tableSectors[i]);
    }
}

/// Fetch contents of file header from disk.
///
/// * `sector` is the disk sector containing the file header.
void
FileHeader::FetchFrom(unsigned sector)
{
    synchDisk->ReadSector(sector, (char *) &raw);

    unsigned numIndirectTables = GetNumIndirectTables();
    for (unsigned i = 0; i < numIndirectTables; i++) {
        synchDisk->ReadSector(raw.tableSectors[i], (char *) &indirectTables[i]);
    }
}

/// Write the modified contents of the file header back to disk.
///
/// * `sector` is the disk sector to contain the file header.
void
FileHeader::WriteBack(unsigned sector)
{
    synchDisk->WriteSector(sector, (char *) &raw);
    
    unsigned numIndirectTables = GetNumIndirectTables();
    for (unsigned i = 0; i < numIndirectTables; i++) {
        synchDisk->WriteSector(raw.tableSectors[i], (char *) &indirectTables[i]);
    }
}

/// Return which disk sector is storing a particular byte within the file.
/// This is essentially a translation from a virtual address (the offset in
/// the file) to a physical address (the sector where the data at the offset
/// is stored).
///
/// * `offset` is the location within the file of the byte in question.
unsigned
FileHeader::ByteToSector(unsigned offset)
{
    unsigned sector = DivRoundDown(offset, SECTOR_SIZE);
    unsigned table = DivRoundDown(sector, NUM_DIRECT);
    unsigned offsetInTable = offset - (table * NUM_DIRECT * SECTOR_SIZE);
    unsigned indexInTable = DivRoundDown(offsetInTable, SECTOR_SIZE);
    unsigned result = indirectTables[table].dataSectors[indexInTable];

    DEBUG(
        'f', 
        "Traslating byte to sector. Offset: %u, total sector: %u, " 
        "indirect table: %u, offsetInTable: %u, index in table: %u, result sector %u\n", 
        offset, sector, table, offsetInTable, indexInTable, result
    );

    ASSERT(offset >= 0 && offset < raw.numBytes);
    ASSERT(table >= 0 && table < GetNumIndirectTables());
    ASSERT(sector >= 0 && sector < GetNumDataSectors());
    ASSERT(indexInTable >= 0 && indexInTable < NUM_DIRECT);

    return result;
}

/// Return the number of bytes in the file.
unsigned
FileHeader::FileLength() const
{
    return raw.numBytes;
}

/// Print the contents of the file header, and the contents of all the data
/// blocks pointed to by the file header.
void
FileHeader::Print(const char *title)
{
    // TODO: Print con indirect tables
    // char *data = new char [SECTOR_SIZE];

    // if (title == nullptr) {
    //     printf("File header:\n");
    // } else {
    //     printf("%s file header:\n", title);
    // }

    // printf("    size: %u bytes\n"
    //        "    block indexes: ",
    //        raw.numBytes);

    // for (unsigned i = 0; i < raw.numSectors; i++) {
    //     printf("%u ", raw.dataSectors[i]);
    // }
    // printf("\n");

    // for (unsigned i = 0, k = 0; i < raw.numSectors; i++) {
    //     printf("    contents of block %u:\n", raw.dataSectors[i]);
    //     synchDisk->ReadSector(raw.dataSectors[i], data);
    //     for (unsigned j = 0; j < SECTOR_SIZE && k < raw.numBytes; j++, k++) {
    //         if (isprint(data[j])) {
    //             printf("%c", data[j]);
    //         } else {
    //             printf("\\%X", (unsigned char) data[j]);
    //         }
    //     }
    //     printf("\n");
    // }
    // delete [] data;
    return;
}

const RawFileHeader *
FileHeader::GetRaw() const
{
    return &raw;
}

unsigned
FileHeader::GetNumDataSectors() 
{
    return DivRoundUp(raw.numBytes, SECTOR_SIZE);
}

unsigned
FileHeader::GetNumIndirectTables()
{
    return DivRoundUp(DivRoundUp(raw.numBytes, SECTOR_SIZE), NUM_DIRECT);
}

bool
FileHeader::ExtendFile(Bitmap *freeMap, unsigned newFileSize){
    unsigned oldFileSize = raw.numBytes;
    unsigned oldDataSectors = GetNumDataSectors();
    unsigned oldNumIndirectTables = GetNumIndirectTables();
    unsigned oldTotalSectors = oldDataSectors + oldNumIndirectTables;

    unsigned newDataSectors = DivRoundUp(newFileSize, SECTOR_SIZE);
    unsigned newNumIndirectTables = DivRoundUp(DivRoundUp(newFileSize, SECTOR_SIZE), NUM_DIRECT);
    unsigned newTotalSectors = newDataSectors + newNumIndirectTables;

    DEBUG('f', "Extending file from %u bytes to %u bytes\n", oldFileSize, newFileSize);

    if (oldDataSectors == newDataSectors) {
        DEBUG('f', "File already has the sectors neccesary\n");
        raw.numBytes = newFileSize;
        return true;
    }

    if (freeMap->CountClear() < newTotalSectors - oldTotalSectors) {
        DEBUG('f', "Not enough space in disk to extend file\n");
        return false;
    }

    ASSERT(newNumIndirectTables <= NUM_INDIRECT);
    ASSERT(newDataSectors <= NUM_DIRECT * NUM_INDIRECT);

    // Create new indirection tables if necessary
    if (oldNumIndirectTables < newNumIndirectTables) {
        for (unsigned i = oldNumIndirectTables; i < newNumIndirectTables; i++) {
            raw.tableSectors[i] = freeMap->Find();
        }
    }

    // Falta mas logica para saber en que sector arrancar
    // Find the index in the table where we should start writing    
    unsigned sector = oldDataSectors;
    unsigned tableNum = DivRoundDown(sector, NUM_DIRECT);
    unsigned offsetInTable = oldFileSize - (tableNum * NUM_DIRECT * SECTOR_SIZE);
    unsigned indexInTable = DivRoundDown(offsetInTable, SECTOR_SIZE);

    // Add missing sectors
    for (unsigned i = sector; i < newDataSectors; i++) {
        unsigned table = DivRoundDown(i, NUM_DIRECT);
        indirectTables[table].dataSectors[indexInTable] = freeMap->Find();
        indexInTable++;
        indexInTable %= NUM_DIRECT;
    }

    raw.numBytes = newFileSize;

    // Everything ok :)
    return true;
}
