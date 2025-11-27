#include "crash/CrashHandler.h"

#include <execinfo.h>
#include <signal.h>
#include <unistd.h>
#include <iostream>

namespace crash
{

    namespace
    {

        /**
         * @brief Signal handler for segmentation faults.
         *
         * Captures the stack trace and prints it to stderr before terminating the application.
         *
         * @param int Signal number (unused).
         */
        void handleSegfault(int)
        {
            void *array[32];
            size_t size = backtrace(array, 32);
            std::cerr << "\n*** SEGMENTATION FAULT – stack trace ***" << std::endl;
            backtrace_symbols_fd(array, size, STDERR_FILENO);
            std::_Exit(EXIT_FAILURE);
        }
    } // namespace

    void installCrashHandler()
    {
        // Register the segmentation fault handler
        signal(SIGSEGV, handleSegfault);
    }

} // namespace crash
