/// Debugging options.
///
/// Copyright (c) 2021 Docentes de la Universidad Nacional de Rosario.
/// All rights reserved.  See `copyright.h` for copyright notice and
/// limitation of liability and disclaimer of warranty provisions.

#ifndef NACHOS_LIB_DEBUGOPTS__HH
#define NACHOS_LIB_DEBUGOPTS__HH


/// Options that control the behavior when printing debugging messages.
struct DebugOpts {
    /// Whether to print a debug message's location along with itself.
    ///
    /// A location refers to the filename and line number.
    bool location;

    /// Whether to print a debug message's function along with itself.
    ///
    /// This is about the caller function's name.
    bool function;

    /// Whether to sleep right after each debug message.
    bool sleep;

    /// Whether to wait for user input right after each debug message.
    bool interactive;

    DebugOpts()
    {
        location = false;
        function = false;
        sleep = false;
        interactive = false;
    }
};


#endif
