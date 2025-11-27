/**
 * @file CrashHandler.h
 * @brief Signal handler for crash reporting and stack traces.
 */

#pragma once

namespace crash
{

    /**
     * @brief Installs signal handlers for crash reporting.
     *
     * Currently handles SIGSEGV (Segmentation Fault) to print a stack trace
     * before exiting.
     */
    void installCrashHandler();

} // namespace crash
