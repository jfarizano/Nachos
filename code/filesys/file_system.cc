/// Routines to manage the overall operation of the file system.  Implements
/// routines to map from textual file names to files.
///
/// Each file in the file system has:
/// * a file header, stored in a sector on disk (the size of the file header
///   data structure is arranged to be precisely the size of 1 disk sector);
/// * a number of data blocks;
/// * an entry in the file system directory.
///
/// The file system consists of several data structures:
/// * A bitmap of free disk sectors (cf. `bitmap.h`).
/// * A directory of file names and file headers.
///
/// Both the bitmap and the directory are represented as normal files.  Their
/// file headers are located in specific sectors (sector 0 and sector 1), so
/// that the file system can find them on bootup.
///
/// The file system assumes that the bitmap and directory files are kept
/// “open” continuously while Nachos is running.
///
/// For those operations (such as `Create`, `Remove`) that modify the
/// directory and/or bitmap, if the operation succeeds, the changes are
/// written immediately back to disk (the two files are kept open during all
/// this time).  If the operation fails, and we have modified part of the
/// directory and/or bitmap, we simply discard the changed version, without
/// writing it back to disk.
///
/// Our implementation at this point has the following restrictions:
///
/// * there is no synchronization for concurrent accesses;
/// * files have a fixed size, set when the file is created;
/// * files cannot be bigger than about 3KB in size;
/// * there is no hierarchical directory structure, and only a limited number
///   of files can be added to the system;
/// * there is no attempt to make the system robust to failures (if Nachos
///   exits in the middle of an operation that modifies the file system, it
///   may corrupt the disk).
///
/// Copyright (c) 1992-1993 The Regents of the University of California.
///               2016-2021 Docentes de la Universidad Nacional de Rosario.
/// All rights reserved.  See `copyright.h` for copyright notice and
/// limitation of liability and disclaimer of warranty provisions.


#include "file_system.hh"
#include "directory.hh"
#include "file_header.hh"
#include "synch_bitmap.hh"
#include "synch_directory.hh"
#include "threads/lock.hh"

#include <stdio.h>
#include <string.h>


/// Sectors containing the file headers for the bitmap of free sectors, and
/// the directory of files.  These file headers are placed in well-known
/// sectors, so that they can be located on boot-up.
static const unsigned FREE_MAP_SECTOR = 0;
static const unsigned DIRECTORY_SECTOR = 1;

/// Initialize the file system.  If `format == true`, the disk has nothing on
/// it, and we need to initialize the disk to contain an empty directory, and
/// a bitmap of free sectors (with almost but not all of the sectors marked
/// as free).
///
/// If `format == false`, we just have to open the files representing the
/// bitmap and the directory.
///
/// * `format` -- should we initialize the disk?
FileSystem::FileSystem(bool format)
{
    DEBUG('f', "Initializing the file system.\n");

    SynchFile *synchFreeMap = nullptr;
    FileHeader *mapH = new FileHeader;

    SynchFile *synchDirectory = nullptr;
    FileHeader *dirH = new FileHeader;

    freeMapLock = new Lock("Freemap lock");
    directoryLock = new Lock("Directory lock");

    if (format) {
        SynchBitmap     *freeMap = new SynchBitmap(NUM_SECTORS, freeMapLock);
        SynchDirectory  *dir     = new SynchDirectory(NUM_DIR_ENTRIES, directoryLock);

        DEBUG('f', "Formatting the file system.\n");

        // First, allocate space for FileHeaders for the directory and bitmap
        // (make sure no one else grabs these!)
        freeMap->Mark(FREE_MAP_SECTOR);
        freeMap->Mark(DIRECTORY_SECTOR);

        // Second, allocate space for the data blocks containing the contents
        // of the directory and bitmap files.  There better be enough space!

        ASSERT(mapH->Allocate(freeMap->GetBitmap(), FREE_MAP_FILE_SIZE));
        ASSERT(dirH->Allocate(freeMap->GetBitmap(), DIRECTORY_FILE_SIZE));

        // Flush the bitmap and directory `FileHeader`s back to disk.
        // We need to do this before we can `Open` the file, since open reads
        // the file header off of disk (and currently the disk has garbage on
        // it!).

        DEBUG('f', "Writing headers back to disk.\n");
        mapH->WriteBack(FREE_MAP_SECTOR);
        dirH->WriteBack(DIRECTORY_SECTOR);

        // OK to open the bitmap and directory files now.
        // The file system operations assume these two files are left open
        // while Nachos is running.

        freeMapFile   = new OpenFile(mapH, synchFreeMap, 0);
        directoryFile = new OpenFile(dirH, synchDirectory, 1);

        // Once we have the files “open”, we can write the initial version of
        // each file back to disk.  The directory at this point is completely
        // empty; but the bitmap has been changed to reflect the fact that
        // sectors on the disk have been allocated for the file headers and
        // to hold the file data for the directory and bitmap.

        DEBUG('f', "Writing bitmap and directory back to disk.\n");
        freeMap->Request();
        freeMap->WriteBack(freeMapFile);     // flush changes to disk
        dir->Request();
        dir->WriteBack(directoryFile);
        DEBUG('f', "Bitmap and directory saved to disk.\n");
        
        if (debug.IsEnabled('f')) {
            freeMap->Print();
            dir->Print();

            delete freeMap;
            delete dir;
        }
    } else {
        // If we are not formatting the disk, just open the files
        // representing the bitmap and directory; these are left open while
        // Nachos is running.
        mapH->FetchFrom(FREE_MAP_SECTOR);
        freeMapFile   = new OpenFile(mapH, synchFreeMap, 0);
        
        dirH->FetchFrom(DIRECTORY_SECTOR);
        directoryFile = new OpenFile(dirH, synchDirectory, 1);
    }

    DEBUG('f', "Creating global open files table\n");
    openFiles = new OpenFilesTable;
    openFiles->AddFile(nullptr, mapH, synchFreeMap);
    openFiles->AddFile(nullptr, dirH, synchDirectory);
    DEBUG('f', "Filesystem initialized\n");
}

FileSystem::~FileSystem()
{
    DEBUG('f', "Deleting filesystem\n");
    this->Close(0);
    this->Close(1);
    delete freeMapFile;
    delete directoryFile;
    delete openFiles;
    delete freeMapLock;
    delete directoryLock;
}

/// Create a file in the Nachos file system (similar to UNIX `create`).
/// Since we cannot increase the size of files dynamically, we have to give
/// `Create` the initial size of the file.
///
/// The steps to create a file are:
/// 1. Make sure the file does not already exist.
/// 2. Allocate a sector for the file header.
/// 3. Allocate space on disk for the data blocks for the file.
/// 4. Add the name to the directory.
/// 5. Store the new file header on disk.
/// 6. Flush the changes to the bitmap and the directory back to disk.
///
/// Return true if everything goes ok, otherwise, return false.
///
/// Create fails if:
/// * file is already in directory;
/// * no free space for file header;
/// * no free entry for file in directory;
/// * no free space for data blocks for the file.
///
/// Note that this implementation assumes there is no concurrent access to
/// the file system!
///
/// * `name` is the name of file to be created.
/// * `initialSize` is the size of file to be created.
bool
FileSystem::Create(const char *name, unsigned initialSize)
{
    DEBUG('f', "Creating file %s, size %u\n", name, initialSize);

    ASSERT(name != nullptr);
    ASSERT(initialSize < MAX_FILE_SIZE);

    SynchDirectory *dir = new SynchDirectory(NUM_DIR_ENTRIES, directoryLock);
    dir->FetchFrom(directoryFile);

    bool success;

    if (dir->Find(name) != -1) {
        DEBUG('f', "File %s already exists\n", name);
        success = false;  // File is already in directory.
    } else {
        SynchBitmap *freeMap = new SynchBitmap(NUM_SECTORS, freeMapLock);
        freeMap->FetchFrom(freeMapFile);
        int sector = freeMap->Find();
          // Find a sector to hold the file header.
        if (sector == -1) {
            DEBUG('f', "No free block for file header, for file %s.\n", name);
            success = false;  // No free block for file header.
        } else if (!dir->Add(name, sector)) {
            DEBUG('f', "No space in directory for file %s.\n", name);
            success = false;  // No space in directory.
        } else {
            FileHeader *h = new FileHeader;
            success = h->Allocate(freeMap->GetBitmap(), initialSize);
              // Fails if no space on disk for data.
            if (success) {
                // Everything worked, flush all changes back to disk.
                h->WriteBack(sector);
                dir->WriteBack(directoryFile);
                freeMap->WriteBack(freeMapFile);
            } else {
                DEBUG('f', "No space on disk for data for file %s.\n", name);
            }
            delete h;
        }

        // Manually release the locks if something went wrong
        if (!success) {
            freeMap->Flush();
        }
        delete freeMap;
    }

    if (!success) {
        dir->Flush();
    }

    delete dir;
    return success;

}

/// Open a file for reading and writing.
///
/// To open a file:
/// 1. Find the location of the file's header, using the directory.
/// 2. Bring the header into memory.
///
/// * `name` is the text name of the file to be opened.
OpenFile *
FileSystem::Open(const char *name)
{
    ASSERT(name != nullptr);

    int fId;
    OpenFile *openFile = nullptr;

    // File wasn't opened by another thread so it's added to the table
    if ((fId = openFiles->Find(name)) == - 1) {
        SynchDirectory *dir = new SynchDirectory(NUM_DIR_ENTRIES, directoryLock);
        
        DEBUG('f', "Opening file %s\n", name);
        dir->FetchFrom(directoryFile);
        int sector = dir->Find(name);

        if (sector >= 0) {
            FileHeader *hdr = new FileHeader;
            hdr->FetchFrom(sector);

            SynchFile *synch = new SynchFile;
            fId = openFiles->AddFile(name, hdr, synch);
            
            if (fId != -1) {
                openFile = new OpenFile(hdr, synch, fId);  // `name` was found in directory.
            } else {
                delete hdr;
                delete synch;
            }
        }
        dir->Flush();
        delete dir;
    } else { // File was opened by another thread
        DEBUG('f', "File %s already opened\n", name);
        FileInfo *fInfo = openFiles->Get(fId);

        if(fInfo->available) {
            DEBUG('f', "Opening file %s (again)\n", name);
            fInfo->nThreads++;
            openFile = new OpenFile(fInfo->hdr, fInfo->synch, fId);
        } else {
            DEBUG('f', "File %s removed by other thread, could not be opened\n", name);
        }
    }

    return openFile;  // Return null if not found.
}

void
FileSystem::Close(int fId) {
    FileInfo *fInfo;

    DEBUG('f', "Closing file with global id %d\n", fId);
    ASSERT((fInfo = openFiles->Get(fId)) != nullptr);
    fInfo->nThreads--;

    if (fInfo->nThreads == 0) {
        if (!fInfo->available) {
            DEBUG('f', "File with global id %d marked to be deleted, deleting\n", fId);
            ASSERT(this->Delete(fInfo->name));
        }
        delete fInfo->hdr;
        delete fInfo->synch;
        openFiles->RemoveFile(fId);
        DEBUG('f', "File with global id %d removed from global files list\n", fId);
    }    
}

/// Delete a file from the file system.
///
/// This requires:
/// 1. Remove it from the directory.
/// 2. Delete the space for its header.
/// 3. Delete the space for its data blocks.
/// 4. Write changes to directory, bitmap back to disk.
///
/// Return true if the file was deleted, false if the file was not in the
/// file system.
///
/// * `name` is the text name of the file to be removed.
bool
FileSystem::Delete(const char *name) {
    DEBUG('f', "Deleting file %s\n", name);

    DEBUG('f', "Deleting file %s. Fetching directory\n", name);
    SynchDirectory *dir = new SynchDirectory(NUM_DIR_ENTRIES, directoryLock);
    dir->FetchFrom(directoryFile);

    int sector = dir->Find(name);
    if (sector == -1) {
       delete dir;
       DEBUG('f', "File %s\n not found, deletedn't", name);
       dir->Flush();
       return false;  // file not found
    }
    DEBUG('f', "Deleting file %s. Fetching file header\n", name);
    FileHeader *fileH = new FileHeader;
    fileH->FetchFrom(sector);

    DEBUG('f', "Deleting file %s. Fetching bitmap\n", name);
    SynchBitmap *freeMap = new SynchBitmap(NUM_SECTORS, freeMapLock);
    freeMap->FetchFrom(freeMapFile);

    DEBUG('f', "Deleting file %s. Removing data blocks\n", name);
    fileH->Deallocate(freeMap->GetBitmap());  // Remove data blocks.
    DEBUG('f', "Deleting file %s. Removing file header block\n", name);
    freeMap->Clear(sector);      // Remove header block.
    DEBUG('f', "Deleting file %s. Removing from directory\n", name);
    dir->Remove(name);

    DEBUG('f', "Deleting file %s. Writing to disk, free map and dir\n", name);
    freeMap->WriteBack(freeMapFile);  // Flush to disk.
    dir->WriteBack(directoryFile);    // Flush to disk.
    delete fileH;
    delete dir;
    delete freeMap;
    DEBUG('f', "File %s deleted\n", name);
    return true;
}

bool
FileSystem::Remove(const char *name)
{
    ASSERT(name != nullptr);

    int fId;

    if ((fId = openFiles->Find(name)) != - 1) {
        FileInfo *fInfo = openFiles->Get(fId);
        fInfo->available = false;
        return true;
    } else {
        return this->Delete(name);
    }    
}

bool
FileSystem::Extend(unsigned id, unsigned newSize)
{
    bool success = false;

    FileInfo *fInfo;
    ASSERT((fInfo = openFiles->Get(id)) != nullptr);

    SynchDirectory *dir = new SynchDirectory(NUM_DIR_ENTRIES, directoryLock);
    dir->FetchFrom(directoryFile);
    
    int sector;
    // Find the header's sector in the directory
    ASSERT((sector = dir->Find(fInfo->name)) >= 2);

    SynchBitmap *freeMap = new SynchBitmap(NUM_SECTORS, freeMapLock);
    freeMap->FetchFrom(freeMapFile);

    FileHeader *hdr = fInfo->hdr;
        
    if (hdr->ExtendFile(freeMap->GetBitmap(), newSize)) {
        hdr->WriteBack(sector);
        freeMap->WriteBack(freeMapFile);
        success = true;
    } else {
        // Restore previous state if failure
        hdr->FetchFrom(sector);
        freeMap->Flush();
    }

    dir->Flush();
    
    delete dir;
    delete freeMap;

    return success;
}

/// List all the files in the file system directory.
void
FileSystem::List()
{
    SynchDirectory *dir = new SynchDirectory(NUM_DIR_ENTRIES, directoryLock);

    dir->FetchFrom(directoryFile);
    dir->List();
    dir->Flush();
    delete dir;
}

/*
static bool
AddToShadowBitmap(unsigned sector, Bitmap *map)
{
    ASSERT(map != nullptr);

    if (map->Test(sector)) {
        DEBUG('f', "Sector %u was already marked.\n", sector);
        return false;
    }
    map->Mark(sector);
    DEBUG('f', "Marked sector %u.\n", sector);
    return true;
}

static bool
CheckForError(bool value, const char *message)
{
    if (!value) {
        DEBUG('f', "Error: %s\n", message);
    }
    return !value;
}

static bool
CheckSector(unsigned sector, Bitmap *shadowMap)
{
    if (CheckForError(sector < NUM_SECTORS,
                      "sector number too big.  Skipping bitmap check.")) {
        return true;
    }
    return CheckForError(AddToShadowBitmap(sector, shadowMap),
                         "sector number already used.");
}

static bool
CheckFileHeader(const RawFileHeader *rh, const RawIndirectionTable *it, unsigned num, Bitmap *shadowMap)
{
    ASSERT(rh != nullptr);

    bool error = false;

    unsigned numSectors = DivRoundUp(rh->numBytes, SECTOR_SIZE);
    
    DEBUG('f', "Checking file header %u.  File size: %u bytes, number of sectors: %u.\n",
          num, rh->numBytes, numSectors);
    
    unsigned indirectTables = DivRoundUp(DivRoundUp(rh->numBytes, SECTOR_SIZE), NUM_DIRECT);
    for (unsigned i = 0; i < indirectTables; i++) {
        error |= CheckSector(rh->tableSectors[i], shadowMap);
    }

    error |= CheckForError(numSectors <= NUM_DIRECT * NUM_INDIRECT,
                           "too many blocks.");
                        
    for (unsigned i = 0, indexInTable = 0; i < numSectors; i++) {
        unsigned table = DivRoundDown(i, NUM_DIRECT);
        unsigned s = it[table].dataSectors[indexInTable];
        indexInTable++;
        indexInTable %= NUM_DIRECT;
        error |= CheckSector(s, shadowMap);
    }
    return error;
}

static bool
CheckBitmaps(const Bitmap *freeMap, const Bitmap *shadowMap)
{
    bool error = false;
    for (unsigned i = 0; i < NUM_SECTORS; i++) {
        DEBUG('f', "Checking sector %u. Original: %u, shadow: %u.\n",
              i, freeMap->Test(i), shadowMap->Test(i));
        error |= CheckForError(freeMap->Test(i) == shadowMap->Test(i),
                               "inconsistent bitmap.");
    }
    return error;
}

static bool
CheckDirectory(const RawDirectory *rd, Bitmap *shadowMap)
{
    ASSERT(rd != nullptr);
    ASSERT(shadowMap != nullptr);

    bool error = false;
    unsigned nameCount = 0;
    const char *knownNames[NUM_DIR_ENTRIES];

    for (unsigned i = 0; i < NUM_DIR_ENTRIES; i++) {
        DEBUG('f', "Checking direntry: %u.\n", i);
        const DirectoryEntry *e = &rd->table[i];

        if (e->inUse) {
            if (strlen(e->name) > FILE_NAME_MAX_LEN) {
                DEBUG('f', "Filename too long.\n");
                error = true;
            }

            // Check for repeated filenames.
            DEBUG('f', "Checking for repeated names.  Name count: %u.\n",
                  nameCount);
            bool repeated = false;
            for (unsigned j = 0; j < nameCount; j++) {
                DEBUG('f', "Comparing \"%s\" and \"%s\".\n",
                      knownNames[j], e->name);
                if (strcmp(knownNames[j], e->name) == 0) {
                    DEBUG('f', "Repeated filename.\n");
                    repeated = true;
                    error = true;
                }
            }
            if (!repeated) {
                knownNames[nameCount] = e->name;
                DEBUG('f', "Added \"%s\" at %u.\n", e->name, nameCount);
                nameCount++;
            }

            // Check sector.
            error |= CheckSector(e->sector, shadowMap);

            // Check file header.
            FileHeader *h = new FileHeader;
            h->FetchFrom(e->sector);
            const RawFileHeader *rh = h->GetRaw();
            const RawIndirectionTable *it = h->GetRawTables();
            error |= CheckFileHeader(rh, it, e->sector, shadowMap);
            delete h;
        }
    }
    return error;
}
*/

bool
FileSystem::Check()
{
/*
    DEBUG('f', "Performing filesystem check\n");
    bool error = false;

    Bitmap *shadowMap = new Bitmap(NUM_SECTORS);
    shadowMap->Mark(FREE_MAP_SECTOR);
    shadowMap->Mark(DIRECTORY_SECTOR);

    DEBUG('f', "Checking bitmap's file header.\n");

    FileHeader *bitH = new FileHeader;
    bitH->FetchFrom(FREE_MAP_SECTOR);
    const RawFileHeader *bitRH = bitH->GetRaw();
    const RawIndirectionTable *bitIT = bitH->GetRawTables();
    DEBUG('f', "  File size: %u bytes, expected %u bytes.\n",
          bitRH->numBytes, FREE_MAP_FILE_SIZE);
    error |= CheckForError(bitRH->numBytes == FREE_MAP_FILE_SIZE,
                           "bad bitmap header: wrong file size.");
    // Raw file header no guarda más el número de sectores
    // error |= CheckForError(bitRH->numSectors == FREE_MAP_FILE_SIZE / SECTOR_SIZE,
    //                        "bad bitmap header: wrong number of sectors.");
    error |= CheckFileHeader(bitRH, bitIT, FREE_MAP_SECTOR, shadowMap);
    delete bitH;

    DEBUG('f', "Checking directory.\n");

    FileHeader *dirH = new FileHeader;
    dirH->FetchFrom(DIRECTORY_SECTOR);
    const RawFileHeader *dirRH = dirH->GetRaw();
    const RawIndirectionTable *dirIT = dirH->GetRawTables();
    error |= CheckFileHeader(dirRH, dirIT, DIRECTORY_SECTOR, shadowMap);
    delete dirH;

    Bitmap *freeMap = new Bitmap(NUM_SECTORS);
    freeMap->FetchFrom(freeMapFile);
    Directory *dir = new Directory(NUM_DIR_ENTRIES);
    const RawDirectory *rdir = dir->GetRaw();
    dir->FetchFrom(directoryFile);
    error |= CheckDirectory(rdir, shadowMap);
    delete dir;

    // The two bitmaps should match.
    DEBUG('f', "Checking bitmap consistency.\n");
    error |= CheckBitmaps(freeMap, shadowMap);
    delete shadowMap;
    delete freeMap;

    DEBUG('f', error ? "Filesystem check failed.\n"
                     : "Filesystem check succeeded.\n");

    return !error;
*/
    return true;
}

/// Print everything about the file system:
/// * the contents of the bitmap;
/// * the contents of the directory;
/// * for each file in the directory:
/// * the contents of the file header;
/// * the data in the file.
void
FileSystem::Print()
{
    FileHeader *bitH    = new FileHeader;
    FileHeader *dirH    = new FileHeader;
    Bitmap     *freeMap = new Bitmap(NUM_SECTORS);
    Directory  *dir     = new Directory(NUM_DIR_ENTRIES);

    printf("--------------------------------\n");
    bitH->FetchFrom(FREE_MAP_SECTOR);
    bitH->Print("Bitmap");

    printf("--------------------------------\n");
    dirH->FetchFrom(DIRECTORY_SECTOR);
    dirH->Print("Directory");

    printf("--------------------------------\n");
    freeMap->FetchFrom(freeMapFile);
    freeMap->Print();

    printf("--------------------------------\n");
    dir->FetchFrom(directoryFile);
    dir->Print();
    printf("--------------------------------\n");

    delete bitH;
    delete dirH;
    delete freeMap;
    delete dir;
}
