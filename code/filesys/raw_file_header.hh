/// Copyright (c) 1992-1993 The Regents of the University of California.
///               2016-2021 Docentes de la Universidad Nacional de Rosario.
/// All rights reserved.  See `copyright.h` for copyright notice and
/// limitation of liability and disclaimer of warranty provisions.

#ifndef NACHOS_FILESYS_RAWFILEHEADER__HH
#define NACHOS_FILESYS_RAWFILEHEADER__HH


#include "machine/disk.hh"

// Maximum number of indirection tables
static const unsigned NUM_INDIRECT
  = (SECTOR_SIZE - 1 * sizeof (int)) / sizeof (int);
// Number of disk sectors with data for each indirection table
static const unsigned NUM_DIRECT
  = SECTOR_SIZE / sizeof (int);
const unsigned MAX_FILE_SIZE = NUM_INDIRECT * NUM_DIRECT * SECTOR_SIZE;

struct RawFileHeader {
    unsigned numBytes;  ///< Number of bytes in the file.
    unsigned tableSectors[NUM_INDIRECT];  ///< Disk sector numbers for each indirection
                                          ///< table in the file.
};

struct RawIndirectionTable {
  unsigned dataSectors[NUM_DIRECT]; /// Disk sector numbers for each data block
                                    // in the file
};

/*
// NOTE:
cant de indirectables = numBytes / SECTOR_SIZE / NUM_INDIRECT; pa rriba
position / NUM_INDIRECT <- Cual tabla de indireccion
position - (position / NUM_INDIRECT) <- posicion dentro de la tabla de indireccion
(position - (position / NUM_INDIRECT) ) / sector_size <- sector de la tabla de indireccion
*/

#endif
