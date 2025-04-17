/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   The code included in this file is provided under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license. Permission
   To use, copy, modify, and/or distribute this software for any purpose with or
   without fee is hereby granted provided that the above copyright notice and
   this permission notice appear in all copies.

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

#include "threads/stl_ChildProcess.h"

#include <threads/stl_StreamProcess.h>

namespace stl {
    ChildProcess::ChildProcess() {
    }

    ChildProcess::~ChildProcess() {
    }

    bool ChildProcess::isRunning() const {
        return activeProcess != nullptr && activeProcess->isRunning();
    }

    int ChildProcess::readProcessOutput(void *dest, int numBytes) {
        return activeProcess != nullptr ? activeProcess->read(dest, numBytes) : 0;
    }

    bool ChildProcess::kill() {
        return activeProcess == nullptr || activeProcess->killProcess();
    }

    uint32 ChildProcess::getExitCode() const {
        return activeProcess != nullptr ? activeProcess->getExitCode() : 0;
    }

    bool ChildProcess::waitForProcessToFinish(const int timeoutMs) const {
        auto timeoutTime = Time::getMillisecondCounter() + (uint32) timeoutMs;

        do {
            if (!isRunning())
                return true;

            Thread::sleep(2);
        } while (timeoutMs < 0 || Time::getMillisecondCounter() < timeoutTime);

        return false;
    }

    String ChildProcess::readAllProcessOutput() {
        MemoryOutputStream result;

        for (;;) {
            char buffer[512];
            auto num = readProcessOutput(buffer, sizeof (buffer));

            if (num <= 0)
                break;

            result.write(buffer, (size_t) num);
        }

        return result.toString();
    }







    StreamProcess::~StreamProcess() noexcept {
        close_fds();
    }

    StreamProcess::id_type StreamProcess::get_id() const noexcept {
        return data.id;
    }

    bool StreamProcess::write(const std::string &str) {
        return write(str.c_str(), str.size());
    }
} // namespace stl
