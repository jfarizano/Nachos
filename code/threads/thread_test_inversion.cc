// Archivo dado por Damián y adaptado a nuestro Nachos
#include "system.hh"
#include "lock.hh"
#include <stdio.h>

Lock* lockInversion;

void High(void* args) {
    lockInversion->Acquire();
    lockInversion->Release();

    printf("High priority task done.\n");
}

void Medium(void* args) {
    printf("Medium priority infinite loop...\n");

    while (1) currentThread->Yield();
}

void Low(void* args) {
    lockInversion->Acquire();
        currentThread->Yield();
    lockInversion->Release();

    printf("Low priority task done.\n");
}

void 
ThreadTestInversion() {
    lockInversion = new Lock("Lock");

    Thread *t4 = new Thread("High", true, 0);
    Thread *t3 = new Thread("Medium 1", false, 2);
    Thread *t2 = new Thread("Medium 2", false, 2);
    Thread *t1 = new Thread("Low", true, 4);

    t1->Fork(Low, nullptr);
    currentThread->Yield();
    t2->Fork(Medium, nullptr);
    t3->Fork(Medium, nullptr);
    t4->Fork(High, nullptr);

    t1->Join();
    t4->Join();
}