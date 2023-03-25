#include "open_files_table.hh"
#include <string.h>

OpenFilesTable::OpenFilesTable()
{
  filesInfoTable = new Table<FileInfo *>;
}

OpenFilesTable::~OpenFilesTable()
{
  // ASSERT(filesInfoTable->FetchCount() == 0);
  delete filesInfoTable;
}

int
OpenFilesTable::AddFile(const char *name, FileHeader *hdr, SynchFile *synch)
{
  FileInfo *fInfo = new FileInfo;
  if (name != nullptr) {
    strncpy(fInfo->name, name, FILE_NAME_MAX_LEN);
  }
  fInfo->hdr = hdr;
  fInfo->synch = synch;
  fInfo->available = true;
  fInfo->nThreads = 1;

  int fId;
  if ((fId = filesInfoTable->Add(fInfo)) == -1 ){
    delete fInfo;
  }
  
  return fId;
}

void 
OpenFilesTable::RemoveFile(int fileId)
{
  FileInfo *fInfo = filesInfoTable->Remove(fileId);
  delete fInfo;
  return;
}

int
OpenFilesTable::Find(const char *name)
{
  DEBUG('f', "Searching file %s on open files table\n", name);

  ASSERT(name != nullptr);

  for (unsigned i = 0; i < filesInfoTable->SIZE; i++) {
      if (filesInfoTable->HasKey(i)
            && !strncmp(filesInfoTable->Get(i)->name, name, FILE_NAME_MAX_LEN)) {
          return i;
      }
  }

  return -1;  // name not in table
}

FileInfo*
OpenFilesTable::Get(int fileId)
{
  return filesInfoTable->Get(fileId);
}

