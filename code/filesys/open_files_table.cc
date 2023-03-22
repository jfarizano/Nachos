#include "open_files_table.hh"
#include <string.h>

OpenFilesTable::OpenFilesTable()
{
  filesInfoTable = new Table<FileInfo *>;
}

OpenFilesTable::~OpenFilesTable()
{
  ASSERT(filesInfoTable->FetchCount() == 0);
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
  // TODO: this
  return -1;
}

FileInfo*
OpenFilesTable::Get(int fileId)
{
  return filesInfoTable->Get(fileId);
}

