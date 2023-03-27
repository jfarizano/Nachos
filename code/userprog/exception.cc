/// Entry points into the Nachos kernel from user programs.
///
/// There are two kinds of things that can cause control to transfer back to
/// here from user code:
///
/// * System calls: the user code explicitly requests to call a procedure in
///   the Nachos kernel.  Right now, the only function we support is `Halt`.
///
/// * Exceptions: the user code does something that the CPU cannot handle.
///   For instance, accessing memory that does not exist, arithmetic errors,
///   etc.
///
/// Interrupts (which can also cause control to transfer from user code into
/// the Nachos kernel) are handled elsewhere.
///
/// For now, this only handles the `Halt` system call.  Everything else core-
/// dumps.
///
/// Copyright (c) 1992-1993 The Regents of the University of California.
///               2016-2021 Docentes de la Universidad Nacional de Rosario.
/// All rights reserved.  See `copyright.h` for copyright notice and
/// limitation of liability and disclaimer of warranty provisions.


#include "transfer.hh"
#include "syscall.h"
#include "filesys/directory_entry.hh"
#include "threads/system.hh"
#include "args.hh"

#include <stdio.h>

void
InitProcess(void *args)
{
    currentThread->space->InitRegisters();  // Set the initial register values.
    currentThread->space->RestoreState();   // Load page table register.    

    if (args != nullptr) {
        // Copio los argumentos en espacio de usuario
        unsigned argc = WriteArgs((char**) args);
        // Guardo la cantidad de args en el registro 4
        machine->WriteRegister(4, argc);
        // WriteArgs deja el stack apuntando al principio de la estructura de args
        int sp = machine->ReadRegister(STACK_REG);
        // Escribo este puntero en el registro 5
        machine->WriteRegister(5, sp);
        // Dejo 24 bytes libres por convención de llamada de MIPS.
        machine->WriteRegister(STACK_REG, sp - 24);
    }

    machine->Run();  // Jump to the user progam.
}


static void
IncrementPC()
{
    unsigned pc;

    pc = machine->ReadRegister(PC_REG);
    machine->WriteRegister(PREV_PC_REG, pc);
    pc = machine->ReadRegister(NEXT_PC_REG);
    machine->WriteRegister(PC_REG, pc);
    pc += 4;
    machine->WriteRegister(NEXT_PC_REG, pc);
}

/// Do some default behavior for an unexpected exception.
///
/// NOTE: this function is meant specifically for unexpected exceptions.  If
/// you implement a new behavior for some exception, do not extend this
/// function: assign a new handler instead.
///
/// * `et` is the kind of exception.  The list of possible exceptions is in
///   `machine/exception_type.hh`.
static void
DefaultHandler(ExceptionType et)
{
    int exceptionArg = machine->ReadRegister(2);

    fprintf(stderr, "Unexpected user mode exception: %s, arg %d.\n",
            ExceptionTypeToString(et), exceptionArg);
    ASSERT(false);
}

/// Handle a system call exception.
///
/// * `et` is the kind of exception.  The list of possible exceptions is in
///   `machine/exception_type.hh`.
///
/// The calling convention is the following:
///
/// * system call identifier in `r2`;
/// * 1st argument in `r4`;
/// * 2nd argument in `r5`;
/// * 3rd argument in `r6`;
/// * 4th argument in `r7`;
/// * the result of the system call, if any, must be put back into `r2`.
///
/// And do not forget to increment the program counter before returning. (Or
/// else you will loop making the same system call forever!)
static void
SyscallHandler(ExceptionType _et)
{
    int scid = machine->ReadRegister(2);

    switch (scid) {

        case SC_HALT:
            DEBUG('e', "Shutdown, initiated by user program.\n");
            interrupt->Halt();
            break;
        
        case SC_EXIT: {
            int status = machine->ReadRegister(4);
            DEBUG('e', "Thread '%s' exiting with status %d\n", currentThread->GetName(), status);

            currentThread->Finish(status);

            break;
        }

        case SC_EXEC: {
            int filenameAddr = machine->ReadRegister(4);
            int argsAddr = machine->ReadRegister(5);
            int joinable = machine->ReadRegister(6);

            if (filenameAddr == 0) {
                DEBUG('e', "Error: address to filename string is null.\n");
                machine->WriteRegister(2, -1);
                break;
            }

            char *filename = new char[FILE_NAME_MAX_LEN + 1];
            if (!ReadStringFromUser(filenameAddr,
                                    filename, sizeof filename * FILE_NAME_MAX_LEN)) {
                DEBUG('e', "Error: filename string too long (maximum is %u bytes).\n",
                      FILE_NAME_MAX_LEN);
                machine->WriteRegister(2, -1);
                break;
            }

            OpenFile *executable = fileSystem->Open(filename);

            if (executable == nullptr) {
                DEBUG('e', "Error: file '%s' could not be opened.\n", filename);
                machine->WriteRegister(2, -1);
                break;
            }

            Thread *t = new Thread(filename, (bool) joinable, currentThread->GetPriority());
            AddressSpace *space = new AddressSpace(executable, t->pid);
            t->space = space;
            
            // De esta forma, al terminar el thread, se liberan el header
            // y la estructura de synch porque se cierra el archivo
            #ifdef USE_SWAP
            t->filesTable->Add(space->swap);
            #endif

            char **args = nullptr;

            if (argsAddr != 0) {
                args = SaveArgs(argsAddr);
            }

            t->Fork(InitProcess, args);

            machine->WriteRegister(2, t->pid);
            break;
        }

        case SC_JOIN: {
            SpaceId id = machine->ReadRegister(4);
            if(!runningThreads->HasKey(id)) {
                DEBUG('e', "Error: Thread not found.\n");
                machine->WriteRegister(2, -1);
            } else {
                DEBUG('e', "`Join` requested for pid `%u`.\n", id);
                Thread *child = runningThreads->Get(id);
                machine->WriteRegister(2, child->Join());
            }
            break;
        }

        case SC_CREATE: {
            int filenameAddr = machine->ReadRegister(4);
            if (filenameAddr == 0) {
                DEBUG('e', "SC_CREATE->Error: address to filename string is null.\n");
                machine->WriteRegister(2, -1);
                break;
            }

            char filename[FILE_NAME_MAX_LEN + 1];
            if (!ReadStringFromUser(filenameAddr,
                                    filename, sizeof filename)) {
                DEBUG('e', "Error: filename string too long (maximum is %u bytes).\n",
                      FILE_NAME_MAX_LEN);
                machine->WriteRegister(2, -1);
                break;
            }

            DEBUG('e', "`Create` requested for file `%s`.\n", filename);

            if (!fileSystem->Create(filename, 0)){
                DEBUG('e', "Error: file '%s' could not be created.\n", filename);
                machine->WriteRegister(2, -1);
                break;
            } else {
                DEBUG('e', "`File `%s` created.\n", filename);
            }

            break;
        }

        case SC_REMOVE: {
            int filenameAddr = machine->ReadRegister(4);

            if (filenameAddr == 0) {
                DEBUG('e', "SC_REMOVE->Error: address to filename string is null.\n");
                machine->WriteRegister(2, -1);
                break;
            }

            char filename[FILE_NAME_MAX_LEN + 1];
            if (!ReadStringFromUser(filenameAddr,
                                    filename, sizeof filename)) {
                DEBUG('e', "Error: filename string too long (maximum is %u bytes).\n",
                      FILE_NAME_MAX_LEN);
                machine->WriteRegister(2, -1);
                break;
            }

            DEBUG('e', "`Remove` requested for file `%s`.\n", filename);

            if (!fileSystem->Remove(filename)){
                DEBUG('e', "Error: file '%s' could not be removed.\n", filename);
                machine->WriteRegister(2, -1);
            } else {
                DEBUG('e', "`File `%s` removed.\n", filename);
                machine->WriteRegister(2, 1);
            }

            break;
        }

        case SC_OPEN: {
            int filenameAddr = machine->ReadRegister(4);

            if (filenameAddr == 0) {
                DEBUG('e', "SC_OPEN->Error: address to filename string is null.\n");
                machine->WriteRegister(2, -1);
                break;
            }

            char filename[FILE_NAME_MAX_LEN + 1];
            if (!ReadStringFromUser(filenameAddr,
                                    filename, sizeof filename)) {
                DEBUG('e', "Error: filename string too long (maximum is %u bytes).\n",
                      FILE_NAME_MAX_LEN);
                machine->WriteRegister(2, -1);
                break;
            }

            DEBUG('e', "`Open` requested for file `%s`.\n", filename);

            OpenFile* file = fileSystem->Open(filename);
            if (!file) {
                DEBUG('e', "Error: file '%s' could not be opened.\n", filename);
                machine->WriteRegister(2, -1);
                break;
            } else {
                OpenFileId fileId = currentThread->filesTable->Add(file);
                
                if (fileId == -1) {
                    // FIXME: Esto puede provocar bugs si no se hace un close
                    // en el filesystem
                    DEBUG('e', "Error: Files table full, can not open more files.\n", filename);
                }

                machine->WriteRegister(2, fileId);
            }

            break;
        }

        case SC_CLOSE: {
            OpenFileId fid = machine->ReadRegister(4);
            DEBUG('e', "`Close` requested for id %u.\n", fid);

            if (currentThread->filesTable->HasKey(fid)) {
                OpenFile *file = currentThread->filesTable->Remove(fid);
                #ifndef FILESYS_STUB
                fileSystem->Close(file->GetGlobalId());
                #endif
                delete file;
                DEBUG('e', "File id %u closed.\n", fid);
                machine->WriteRegister(2, fid);
            } else {
                DEBUG('e', "Error closing file: id %u does not exist", fid);
                machine->WriteRegister(2, -1);
            }

            break;
        }

        case SC_READ: {          
            int userAddress = machine->ReadRegister(4);
            int size = machine->ReadRegister(5);
            OpenFileId fid = machine->ReadRegister(6);
            char buffer[size];
            int bytesRead = 0;
            
            if (size <= 0) {
                DEBUG('e', "Error: Invalid size.\n");
                machine->WriteRegister(2, -1);
                break;
            } else if (fid == CONSOLE_OUTPUT || fid < 0) {
                DEBUG('e', "Error: Invalid file for reading.\n");
                machine->WriteRegister(2, -1);
                break;
            } else if(fid == CONSOLE_INPUT) {
                DEBUG('e', "Reading from console.\n");
                synchConsole->ReadBuffer(buffer, size);
                bytesRead = size;
                WriteBufferToUser(buffer, userAddress, bytesRead);
            } else {
                if (currentThread->filesTable->HasKey(fid)) {
                    DEBUG('e', "Reading from file with id %u.\n", fid);
                    OpenFile *file = currentThread->filesTable->Get(fid);
                    bytesRead = file->Read(buffer, size);
                    if (bytesRead > 0) {
                        WriteBufferToUser(buffer, userAddress, bytesRead);
                    }
                } else {
                    DEBUG('e', "File with id %u does not exists.\n", fid);
                    machine->WriteRegister(2, -1);
                    break;
                }
            }
            
            machine->WriteRegister(2, bytesRead);

            break;
        }

        case SC_WRITE: {
            int userAddress = machine->ReadRegister(4);
            int size = machine->ReadRegister(5);
            OpenFileId fid = machine->ReadRegister(6);
            char buffer[size];
            int bytesWritten = 0;

            if (size <= 0) {
                DEBUG('e', "Error: Invalid size.\n");
                machine->WriteRegister(2, -1);
                break;
            } else if (fid <= CONSOLE_INPUT) {
                DEBUG('e', "Error: Invalid file for writing.\n");
                machine->WriteRegister(2, -1);
                break;
            } else if (fid == CONSOLE_OUTPUT) {
                DEBUG('e', "Writing to console.\n");
                ReadBufferFromUser(userAddress, buffer, size);
                synchConsole->WriteBuffer(buffer, size);
                bytesWritten = size;
            } else {
                if (currentThread->filesTable->HasKey(fid)) {
                    DEBUG('e', "Writing to file with id %u.\n", fid);
                    OpenFile *file = currentThread->filesTable->Get(fid);
                    ReadBufferFromUser(userAddress, buffer, size);
                    bytesWritten = file->Write(buffer, size);
                } else {
                    DEBUG('e', "File with id %u does not exists.\n", fid);
                    machine->WriteRegister(2, -1);
                    break;
                }
            }

            machine->WriteRegister(2, bytesWritten);

            break;
        }

        case SC_PS: {
            scheduler->Print();
            break;
        }
        
        default:
            fprintf(stderr, "Unexpected system call: id %d.\n", scid);
            ASSERT(false);

    }

    IncrementPC();
}

#ifdef VMEM
static void
PageFaultHandler(ExceptionType _et)
{
    int badVAddr = machine->ReadRegister(BAD_VADDR_REG);
    unsigned vpn = badVAddr / PAGE_SIZE;
    
    DEBUG('e', "Page fault in thread %s, virtual page %u, badVAddr %u\n",
         currentThread->GetName(), vpn, badVAddr);

    TranslationEntry *entry = currentThread->space->GetTranslationEntry(vpn);

    if (!entry->valid) {
        DEBUG('e', "Page not found in memory\n");
        currentThread->space->LoadPage(vpn);
    }
    
    static int circularIndex = 0;
    TranslationEntry *tlb = machine->GetMMU()->tlb;

    if (tlb[circularIndex].valid) {
        unsigned victimVpn = tlb[circularIndex].virtualPage;
        TranslationEntry *victimEntry = currentThread->space->GetTranslationEntry(victimVpn);

        victimEntry->use = tlb[circularIndex].use;
        victimEntry->dirty = tlb[circularIndex].dirty;
    }

    tlb[circularIndex].virtualPage = vpn;
    tlb[circularIndex].physicalPage = entry->physicalPage;
    tlb[circularIndex].valid = entry->valid;
    tlb[circularIndex].readOnly = entry->readOnly;
    tlb[circularIndex].use = entry->use;
    tlb[circularIndex].dirty = entry->dirty;

    circularIndex++;
    circularIndex %= TLB_SIZE;
    stats->numPageFaults++;
}
#endif

static void
ReadOnlyHandler(ExceptionType _et)
{   
    fprintf(stderr, "Cannot write on page marked as read only :'(\n");
    ASSERT(false);
    return;
}

/// By default, only system calls have their own handler.  All other
/// exception types are assigned the default handler.
void
SetExceptionHandlers()
{
    machine->SetHandler(NO_EXCEPTION,            &DefaultHandler);
    machine->SetHandler(SYSCALL_EXCEPTION,       &SyscallHandler);
    #ifdef VMEM
    machine->SetHandler(PAGE_FAULT_EXCEPTION,    &PageFaultHandler);
    #else
    machine->SetHandler(PAGE_FAULT_EXCEPTION,    &DefaultHandler);
    #endif
    machine->SetHandler(READ_ONLY_EXCEPTION,     &ReadOnlyHandler);
    machine->SetHandler(BUS_ERROR_EXCEPTION,     &DefaultHandler);
    machine->SetHandler(ADDRESS_ERROR_EXCEPTION, &DefaultHandler);
    machine->SetHandler(OVERFLOW_EXCEPTION,      &DefaultHandler);
    machine->SetHandler(ILLEGAL_INSTR_EXCEPTION, &DefaultHandler);
}