/// Copyright (c) 1992-1993 The Regents of the University of California.
///               2007-2009 Universidad de Las Palmas de Gran Canaria.
///               2016-2021 Docentes de la Universidad Nacional de Rosario.
/// All rights reserved.  See `copyright.h` for copyright notice and
/// limitation of liability and disclaimer of warranty provisions.

#include "thread_test_prod_cons.hh"
#include "system.hh"
#include "condition.hh"

#include <stdio.h>
#include <unistd.h>


static const unsigned N = 10;

Lock *lockProdCons = new Lock("lock");
Condition *condEmpty = new Condition("condition empty", lockProdCons);
Condition *condFull = new Condition("condition full", lockProdCons);

int buffer = 0;

static void
productor(void* arg)
{
  while(1) {
    currentThread->Yield();
    printf("%s arrancando\n", currentThread->GetName());
    lockProdCons->Acquire();
    currentThread->Yield();
    while(buffer == N) {
      condFull->Wait();
    }
    currentThread->Yield();
    printf("%s produciendo\n", currentThread->GetName());
    buffer++;
    sleep(1);
    currentThread->Yield();
    condEmpty->Signal();
    currentThread->Yield();
    lockProdCons->Release();
    currentThread->Yield();
  }
}

static void
consumidor(void* arg)
{
  while(1) {
    currentThread->Yield();
    printf("%s arrancando\n", currentThread->GetName());
    lockProdCons->Acquire();
    currentThread->Yield();
    while(buffer == 0) {
        condEmpty->Wait();
    }
    currentThread->Yield();
    printf("%s consumiendo\n", currentThread->GetName());
    buffer--;
    sleep(1);
    currentThread->Yield();
    condFull->Signal();
    currentThread->Yield();
    lockProdCons->Release();
    currentThread->Yield();
  }
}

void
ThreadTestProdCons()
{
  char namesProds[5][16]; // Pasaba algo raro con los nombres si no se hacía esto
  List<Thread *> *productores = new List<Thread *>;
  for (int i = 0; i < 5; i++) {
      sprintf(namesProds[i], "Productor %u", i);
      printf("Creando: %s \n", namesProds[i]);
      Thread *t = new Thread(namesProds[i], true, 2);
      t->Fork(productor, nullptr);
      productores->Append(t);
  }

  char namesCons[5][16]; // Pasaba algo raro con los nombres si no se hacía esto
  List<Thread *> *consumidores = new List<Thread *>;
  for (int i = 0; i < 5; i++) {
    sprintf(namesCons[i], "Consumidor %u", i);
    printf("Creando: %s \n", namesCons[i]);
    Thread *t = new Thread(namesCons[i], true, 2);
    t->Fork(consumidor, nullptr);
    consumidores->Append(t);
  }

  
  Thread *t;
  while (!productores->IsEmpty()) {
        t = productores->Pop();
        t->Join();
  }

  while (!consumidores->IsEmpty()) {
        t = consumidores->Pop();
        t->Join();
  }

}
