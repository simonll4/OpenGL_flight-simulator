#include "crash/CrashHandler.h"

#include <execinfo.h>
#include <signal.h>
#include <unistd.h>
#include <iostream>

namespace crash {

namespace {
void handleSegfault(int) {
    void* array[32];
    size_t size = backtrace(array, 32);
    std::cerr << "\n*** SEGMENTATION FAULT – stack trace ***" << std::endl;
    backtrace_symbols_fd(array, size, STDERR_FILENO);
    std::_Exit(EXIT_FAILURE);
}
} // namespace

void installCrashHandler() {
    signal(SIGSEGV, handleSegfault);
}

} // namespace crash
