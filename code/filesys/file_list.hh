#ifndef NACHOS_FILESYS_FILEINFO__HH
#define NACHOS_FILESYS_FILEINFO__HH

#include "open_file.hh"

struct FileInfo
{
  char* name;
  OpenFile* masterFile;
  bool available;
  unsigned nThreads;
};

class OpenFilesList {
public:
    OpenFilesList();

    ~OpenFilesList();

    FileInfo* Find(char *name);

    

private:
    List<FileInfo*> *filesInfoList;
};

#endif