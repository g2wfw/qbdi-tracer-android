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

#include "../system/stl_PlatformDefs.h"
#include "../stl_core.h"
#include "stl_Atomic.h"

namespace stl {

//==============================================================================
/**
    Embedding an instance of this class inside another class can be used as a low-overhead
    way of detecting leaked instances.

    This class keeps an internal static count of the number of instances that are
    active, so that when the app is shutdown and the static destructors are called,
    it can check whether there are any left-over instances that may have been leaked.

    To use it, use the STL_LEAK_DETECTOR macro as a simple way to put one in your
    class declaration. Have a look through the juce codebase for examples, it's used
    in most of the classes.

    @tags{Core}
*/
    template<class OwnerClass>
    class LeakedObjectDetector {
    public:
        //==============================================================================
        LeakedObjectDetector() noexcept { ++(getCounter().numObjects); }

        LeakedObjectDetector(const LeakedObjectDetector &) noexcept { ++(getCounter().numObjects); }

        LeakedObjectDetector &operator=(const LeakedObjectDetector &) noexcept = default;

        ~LeakedObjectDetector() {
            if (--(getCounter().numObjects) < 0) {
                DBG ("*** Dangling pointer deletion! Class: " << getLeakedObjectClassName());

                /** If you hit this, then you've managed to delete more instances of this class than you've
                    created.. That indicates that you're deleting some dangling pointers.

                    Note that although this assertion will have been triggered during a destructor, it might
                    not be this particular deletion that's at fault - the incorrect one may have happened
                    at an earlier point in the program, and simply not been detected until now.

                    Most errors like this are caused by using old-fashioned, non-RAII techniques for
                    your object management. Tut, tut. Always, always use std::unique_ptrs, OwnedArrays,
                    ReferenceCountedObjects, etc, and avoid the 'delete' operator at all costs!
                */
                jassertfalse;
            }
        }

    private:
        //==============================================================================
        class LeakCounter {
        public:
            LeakCounter() = default;

            ~LeakCounter() {
                if (numObjects.value > 0) {
                    DBG ("*** Leaked objects detected: " << numObjects.value << " instance(s) of class "
                                                         << getLeakedObjectClassName());

                    /** If you hit this, then you've leaked one or more objects of the type specified by
                        the 'OwnerClass' template parameter - the name should have been printed by the line above.

                        If you're leaking, it's probably because you're using old-fashioned, non-RAII techniques for
                        your object management. Tut, tut. Always, always use std::unique_ptrs, OwnedArrays,
                        ReferenceCountedObjects, etc, and avoid the 'delete' operator at all costs!
                    */
                    jassertfalse;
                }
            }

            Atomic<int> numObjects;
        };

        static const char *getLeakedObjectClassName() {
            return OwnerClass::getLeakedObjectClassName();
        }

        static LeakCounter &getCounter() noexcept {
            static LeakCounter counter;
            return counter;
        }
    };


} // namespace stl
