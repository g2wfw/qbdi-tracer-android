#pragma once/*
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

#include "../text/stl_String.h"
#include "../text/stl_StringRef.h"
#include "../text/stl_StringArray.h"

namespace stl {

//==============================================================================
/** Represents the current executable's process.

    This contains methods for controlling the current application at the
    process-level.

    @see Thread, STLApplicationBase

    @tags{Core}
*/
    class STL_EXPORT Process {
    public:
        //==============================================================================
        enum ProcessPriority {
            LowPriority = 0,
            NormalPriority = 1,
            HighPriority = 2,
            RealtimePriority = 3
        };

        /** Changes the current process's priority.

            @param priority     the process priority, where
                                0=low, 1=normal, 2=high, 3=realtime
        */
        static void STL_CALLTYPE setPriority(ProcessPriority priority);

        /** Kills the current process immediately.

            This is an emergency process terminator that kills the application
            immediately - it's intended only for use only when something goes
            horribly wrong.

            @see STLApplicationBase::quit
        */
        static void STL_CALLTYPE terminate();


        //==============================================================================
        /** Raises the current process's privilege level.

            Does nothing if this isn't supported by the current OS, or if process
            privilege level is fixed.
        */
        static void STL_CALLTYPE raisePrivilege();

        /** Lowers the current process's privilege level.

            Does nothing if this isn't supported by the current OS, or if process
            privilege level is fixed.
        */
        static void STL_CALLTYPE lowerPrivilege();

        //==============================================================================
        /** Returns true if this process is being hosted by a debugger. */
        static bool STL_CALLTYPE isRunningUnderDebugger() noexcept;


        //==============================================================================
        /** Tries to launch the OS's default reader application for a given file or URL. */
        static bool STL_CALLTYPE openDocument(const String &documentURL, const String &parameters);

        /** Tries to launch the OS's default email application to let the user create a message. */
        static bool STL_CALLTYPE openEmailWithAttachments(const String &targetEmailAddress,
                                                          const String &emailSubject,
                                                          const String &bodyText,
                                                          const StringArray &filesToAttach);

        //==============================================================================
#if STL_WINDOWS || DOXYGEN
        /** WINDOWS ONLY - This returns the HINSTANCE of the current module.

            The return type is a void* to avoid being dependent on windows.h - just cast
            it to a HINSTANCE to use it.

            In a normal STL application, this will be automatically set to the module
            handle of the executable.

            If you've built a DLL and plan to use any STL messaging or windowing classes,
            you'll need to make sure you call the setCurrentModuleInstanceHandle()
            to provide the correct module handle in your DllMain() function, because
            the system relies on the correct instance handle when opening windows.
        */
        static void* STL_CALLTYPE getCurrentModuleInstanceHandle() noexcept;

        /** WINDOWS ONLY - Sets a new module handle to be used by the library.

            The parameter type is a void* to avoid being dependent on windows.h, but it actually
            expects a HINSTANCE value.

            @see getCurrentModuleInstanceHandle()
        */
        static void STL_CALLTYPE setCurrentModuleInstanceHandle (void* newHandle) noexcept;
#endif

        //==============================================================================
#if (STL_MAC && STL_MODULE_AVAILABLE_stl_gui_basics) || DOXYGEN
        /** OSX ONLY - Shows or hides the OSX dock icon for this app. */
        static void setDockIconVisible (bool isVisible);
#endif

        //==============================================================================
#if STL_MAC || STL_LINUX || STL_BSD || DOXYGEN

        /** UNIX ONLY - Attempts to use setrlimit to change the maximum number of file
            handles that the app can open. Pass 0 or less as the parameter to mean
            'infinite'. Returns true if it succeeds.
        */
        static bool setMaxNumberOfFileHandles(int maxNumberOfFiles) noexcept;

#endif

    private:
        Process();
        STL_DECLARE_NON_COPYABLE (Process)
    };

} // namespace stl
