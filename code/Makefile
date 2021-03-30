# Copyright (c) 1992      The Regents of the University of California.
#               2016-2021 Docentes de la Universidad Nacional de Rosario.
# All rights reserved.  See `copyright.h` for copyright notice and
# limitation of liability and disclaimer of warranty provisions.

MAKE = make
LPR  = lpr
SH   = bash

.PHONY: all clean test print

all:
	@echo ":: Making $$(tput bold)threads$$(tput sgr0)"
	@$(MAKE) -C threads depend
	@$(MAKE) -C threads all
	@echo ":: Making $$(tput bold)userprog$$(tput sgr0)"
	@$(MAKE) -C userprog depend
	@$(MAKE) -C userprog all
	@echo ":: Making $$(tput bold)vmem$$(tput sgr0)"
	@$(MAKE) -C vmem depend
	@$(MAKE) -C vmem all
	@echo ":: Making $$(tput bold)filesys$$(tput sgr0)"
	@$(MAKE) -C filesys depend
	@$(MAKE) -C filesys all
	@echo ":: Making $$(tput bold)network$$(tput sgr0)"
	@$(MAKE) -C network depend
	@$(MAKE) -C network all
	@echo ":: Making $$(tput bold)bin$$(tput sgr0)"
	@$(MAKE) -C bin
	@echo ":: Making $$(tput bold)userland$$(tput sgr0)"
	@$(MAKE) -C userland

# Do not delete executables in `userland` in case there is no cross-compiler.
clean:
	@echo ":: Cleaning everything"
	@$(MAKE) -C threads clean
	@$(MAKE) -C userprog clean
	@$(MAKE) -C vmem clean
	@$(MAKE) -C filesys clean
	@$(MAKE) -C network clean
	@$(MAKE) -C bin clean
	@$(MAKE) -C userland clean

test:
	@./tests/check.sh

print:
	$(SH) -c '$(LPR) Makefile* */Makefile                              \
	                 threads/*.h threads/*.hh threads/*.cc threads/*.s \
	                 userprog/*.h userprog/*.hh userprog/*.cc          \
	                 filesys/*.hh filesys/*.cc                         \
	                 network/*.hh network/*.cc                         \
	                 machine/*.hh machine/*.cc                         \
	                 bin/noff.h bin/coff.h bin/coff2noff.c             \
	                 userland/*.h userland/*.c userland/*.s'
