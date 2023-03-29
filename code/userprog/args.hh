/// Move command-line arguments between user memory and kernel memory
///
/// These functions follow the standard style of passing arguments in
/// Unix-like operating systems and the C programming language.
///
/// Copyright (c) 2018-2021 Docentes de la Universidad Nacional de Rosario.
/// All rights reserved.  See `copyright.h` for copyright notice and
/// limitation of liability and disclaimer of warranty provisions.

#ifndef NACHOS_USERPROG_ARGS__HH
#define NACHOS_USERPROG_ARGS__HH


/// Save command-line arguments from the memory of a user process.
///
/// It moves an `argv`-like array from user memory to kernel memory, and
/// returns a pointer to the latter.
///
/// Parameters:
/// * `address` is a user-space address pointing to the start of an
///   `argv`-like array.
char **SaveArgs(int address);

/// Write command-line arguments into the stack memory of a user process.
///
/// Considering two example arguments `hello` and `Nachos`, the resulting
/// stack layout is as follows:
///
///                 +┌───────────────────────┐
///          7 bytes │        hello\0        │<───────────────┐ 1st arg.
///                  ├───────────────────────┤                │
///          6 bytes │       Nachos\0        │<───┐ 2nd arg.  │
///                  ├───────────────────────┤    │           │
///     0 to 3 bytes │       (padding)       │    │           │
///                  ├───────────────────────┤    │           │
///          4 bytes │    (null pointer)     │    │           │
///                  ├───────────────────────┤    │           │
///          4 bytes │ (pointer to 2nd arg.) │────┘           │
///                  ├───────────────────────┤                │
///          4 bytes │ (pointer to 1st arg.) │────────────────┘
///                 -└───────────────────────┘<==== sp
///
/// CAUTION: if you intend to use this to pass command-line arguments to a
/// new process in the C way (by passing function arguments to the starting
/// function), then make sure to take into account the function call argument
/// area.  This consists of some space at the top of the stack that has to be
/// reserved when making a function call with arguments; the called function
/// can use the area to store arguments passed in registers.  This is
/// mandated by the MIPS ABI, and its omission may cause corruption of
/// either `argv` or stored register values.  The area must be able to hold
/// the 4 arguments that can be passed in registers: `a0..4`, hence it
/// occupies 16 bytes.  Therefore, the problem is solved by substracting 24
/// to `sp`.
///
/// NOTE: if you pass command-line arguments in some other way (for example,
/// via a system call), then there is no need to reserve the aforementioned
/// area.
///
/// Parameters:
/// * `args` is a kernel-space pointer to the start of an `argv`-like array.
///
/// Returns the count of arguments, not including the trailing null (the
/// same as `argc`).  Frees everything allocated by `SaveArgs`.
unsigned WriteArgs(char **args);


#endif
