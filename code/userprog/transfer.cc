/// Copyright (c) 2019-2021 Docentes de la Universidad Nacional de Rosario.
/// All rights reserved.  See `copyright.h` for copyright notice and
/// limitation of liability and disclaimer of warranty provisions.


#include "transfer.hh"
#include "lib/utility.hh"
#include "threads/system.hh"

#ifdef USE_TLB
static const int MAX_MEM_TRIES = 4;
#else
static const int MAX_MEM_TRIES = 1;
#endif

void ReadBufferFromUser(int userAddress, char *outBuffer,
                        unsigned byteCount)
{
    ASSERT(userAddress != 0);
    ASSERT(outBuffer != nullptr);
    ASSERT(byteCount != 0);

    unsigned count = 0;
    do {
        int temp;
        count++;
        int tries = 0;
        for (; tries < MAX_MEM_TRIES && !machine->ReadMem(userAddress, 1, &temp); tries++){};
        if (tries == MAX_MEM_TRIES) {
            ASSERT(false);
        }
        userAddress++;
        *outBuffer = (unsigned char) temp;
        outBuffer++;
    } while (count < byteCount);

}

bool ReadStringFromUser(int userAddress, char *outString,
                        unsigned maxByteCount)
{
    ASSERT(userAddress != 0);
    ASSERT(outString != nullptr);
    ASSERT(maxByteCount != 0);

    unsigned count = 0;
    do {
        int temp;
        count++;
        int tries = 0;
        for (; tries < MAX_MEM_TRIES && !machine->ReadMem(userAddress, 1, &temp); tries++){};
        if (tries == MAX_MEM_TRIES) {
            ASSERT(false);
        }
        userAddress++;
        *outString = (unsigned char) temp;
    } while (*outString++ != '\0' && count < maxByteCount);

    return *(outString - 1) == '\0';
}

void WriteBufferToUser(const char *buffer, int userAddress,
                       unsigned byteCount)
{
    ASSERT(buffer != nullptr);
    ASSERT(userAddress != 0);
    ASSERT(byteCount != 0);

    unsigned count = 0;
    do {
        int temp = (int) buffer[count++];
        int tries = 0;
        for (; tries < MAX_MEM_TRIES && !machine->WriteMem(userAddress, 1, temp); tries++){};
        if (tries == MAX_MEM_TRIES) {
            ASSERT(false);
        }
        userAddress++;
    } while (count < byteCount);
}

void WriteStringToUser(const char *string, int userAddress)
{
    ASSERT(string != nullptr);
    ASSERT(userAddress != 0);

    do {
        int temp = (int) *string;
        int tries = 0;
        for (; tries < MAX_MEM_TRIES && !machine->WriteMem(userAddress, 1, temp); tries++){};
        if (tries == MAX_MEM_TRIES) {
            ASSERT(false);
        }
        userAddress++;
    } while (*string++ != '\0');
}
