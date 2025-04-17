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

namespace stl
{

//==============================================================================
/**
STL_DECLARE_SINGLETON宏用于管理静态指针
到单例实例。
您通常不会直接使用它，但请参阅宏STL_DECLARE_SINGLETON，
STL_clare_SINGLETON_SINGLETHREADED、STL_clare _singleto_SINGLETHREADED_MINIMAL，
和STL_ IMPLEMENT_。

    @tags{Core}
*/
template <typename Type, typename MutexType, bool onlyCreateOncePerRun>
struct STL_API SingletonHolder  : private MutexType // (inherited so we can use the empty-base-class optimisation)
{
    SingletonHolder() = default;

    ~SingletonHolder()
    {
        /* The static singleton holder is being deleted before the object that it holds
           has been deleted. This could mean that you've forgotten to call clearSingletonInstance()
           in the class's destructor, or have failed to delete it before your app shuts down.
           If you're having trouble cleaning up your singletons, perhaps consider using the
           SharedResourcePointer class instead.
        */
        jassert (instance.load() == nullptr);
    }

    /** Returns the current instance, or creates a new instance if there isn't one. */
    Type* get()
    {
        if (auto* ptr = instance.load())
            return ptr;

        typename MutexType::ScopedLockType sl (*this);

        if (auto* ptr = instance.load())
            return ptr;

        auto once = onlyCreateOncePerRun; // (local copy avoids VS compiler warning about this being constant)

        if (once)
        {
            static bool createdOnceAlready = false;

            if (createdOnceAlready)
            {
                // This means that the doNotRecreateAfterDeletion flag was set
                // and you tried to create the singleton more than once.
                jassertfalse;
                return nullptr;
            }

            createdOnceAlready = true;
        }

        static bool alreadyInside = false;

        if (alreadyInside)
        {
            // This means that your object's constructor has done something which has
            // ended up causing a recursive loop of singleton creation.
            jassertfalse;
            return nullptr;
        }

        const ScopedValueSetter<bool> scope (alreadyInside, true);
        return getWithoutChecking();
    }

    /** Returns the current instance, or creates a new instance if there isn't one, but doesn't do
        any locking, or checking for recursion or error conditions.
    */
    Type* getWithoutChecking()
    {
        if (auto* p = instance.load())
            return p;

        auto* newObject = new Type(); // (create into a local so that instance is still null during construction)
        instance.store (newObject);
        return newObject;
    }

    /** Deletes and resets the current instance, if there is one. */
    void deleteInstance()
    {
        typename MutexType::ScopedLockType sl (*this);
        delete instance.exchange (nullptr);
    }

    /** Called by the class's destructor to clear the pointer if it is currently set to the given object. */
    void clear (Type* expectedObject) noexcept
    {
        instance.compare_exchange_strong (expectedObject, nullptr);
    }

    // This must be atomic, otherwise a late call to get() may attempt to read instance while it is
    // being modified by the very first call to get().
    std::atomic<Type*> instance { nullptr };
};


//==============================================================================
/**
    Macro to generate the appropriate methods and boilerplate for a singleton class.

    To use this, add the line STL_DECLARE_SINGLETON (MyClass, doNotRecreateAfterDeletion)
    to the class's definition.

    Then put a macro STL_IMPLEMENT_SINGLETON (MyClass) along with the class's
    implementation code.

    It's also a very good idea to also add the call clearSingletonInstance() in your class's
    destructor, in case it is deleted by other means than deleteInstance()

    Clients can then call the static method MyClass::getInstance() to get a pointer
    to the singleton, or MyClass::getInstanceWithoutCreating() which will return nullptr if
    no instance currently exists.

    e.g. @code

        struct MySingleton
        {
            MySingleton() {}

            ~MySingleton()
            {
                // this ensures that no dangling pointers are left when the
                // singleton is deleted.
                clearSingletonInstance();
            }

            STL_DECLARE_SINGLETON (MySingleton, false)
        };

        // ..and this goes in a suitable .cpp file:
        STL_IMPLEMENT_SINGLETON (MySingleton)


        // example of usage:
        auto* m = MySingleton::getInstance(); // creates the singleton if there isn't already one.

        ...

        MySingleton::deleteInstance(); // safely deletes the singleton (if it's been created).

    @endcode

    If doNotRecreateAfterDeletion = true, it won't allow the object to be created more
    than once during the process's lifetime - i.e. after you've created and deleted the
    object, getInstance() will refuse to create another one. This can be useful to stop
    objects being accidentally re-created during your app's shutdown code.

    If you know that your object will only be created and deleted by a single thread, you
    can use the slightly more efficient STL_DECLARE_SINGLETON_SINGLETHREADED macro instead
    of this one.

    @see STL_IMPLEMENT_SINGLETON, STL_DECLARE_SINGLETON_SINGLETHREADED
*/
#define STL_DECLARE_SINGLETON(Classname, doNotRecreateAfterDeletion) \
\
    static stl::SingletonHolder<Classname, stl::CriticalSection, doNotRecreateAfterDeletion> singletonHolder; \
    friend stl::SingletonHolder<Classname, stl::CriticalSection, doNotRecreateAfterDeletion>; \
\
    static Classname* STL_CALLTYPE getInstance()                           { return singletonHolder.get(); } \
    static Classname* STL_CALLTYPE getInstanceWithoutCreating() noexcept   { return singletonHolder.instance; } \
    static void STL_CALLTYPE deleteInstance() noexcept                     { singletonHolder.deleteInstance(); } \
    void clearSingletonInstance() noexcept                                  { singletonHolder.clear (this); }


//==============================================================================
/** This is a counterpart to the STL_DECLARE_SINGLETON macros.

    After adding the STL_DECLARE_SINGLETON to the class definition, this macro has
    to be used in the cpp file.
*/
#define STL_IMPLEMENT_SINGLETON(Classname) \
\
    decltype (Classname::singletonHolder) Classname::singletonHolder;


//==============================================================================
/**
    Macro to declare member variables and methods for a singleton class.

    This is exactly the same as STL_DECLARE_SINGLETON, but doesn't use a critical
    section to make access to it thread-safe. If you know that your object will
    only ever be created or deleted by a single thread, then this is a
    more efficient version to use.

    If doNotRecreateAfterDeletion = true, it won't allow the object to be created more
    than once during the process's lifetime - i.e. after you've created and deleted the
    object, getInstance() will refuse to create another one. This can be useful to stop
    objects being accidentally re-created during your app's shutdown code.

    See the documentation for STL_DECLARE_SINGLETON for more information about
    how to use it. Just like STL_DECLARE_SINGLETON you need to also have a
    corresponding STL_IMPLEMENT_SINGLETON statement somewhere in your code.

    @see STL_IMPLEMENT_SINGLETON, STL_DECLARE_SINGLETON, STL_DECLARE_SINGLETON_SINGLETHREADED_MINIMAL
*/
#define STL_DECLARE_SINGLETON_SINGLETHREADED(Classname, doNotRecreateAfterDeletion) \
\
    static stl::SingletonHolder<Classname, stl::DummyCriticalSection, doNotRecreateAfterDeletion> singletonHolder; \
    friend decltype (singletonHolder); \
\
    static Classname* STL_CALLTYPE getInstance()                           { return singletonHolder.get(); } \
    static Classname* STL_CALLTYPE getInstanceWithoutCreating() noexcept   { return singletonHolder.instance; } \
    static void STL_CALLTYPE deleteInstance() noexcept                     { singletonHolder.deleteInstance(); } \
    void clearSingletonInstance() noexcept                                  { singletonHolder.clear (this); }


//==============================================================================
/**
    Macro to declare member variables and methods for a singleton class.

    This is like STL_DECLARE_SINGLETON_SINGLETHREADED, but doesn't do any checking
    for recursion or repeated instantiation. It's intended for use as a lightweight
    version of a singleton, where you're using it in very straightforward
    circumstances and don't need the extra checking.

    See the documentation for STL_DECLARE_SINGLETON for more information about
    how to use it. Just like STL_DECLARE_SINGLETON you need to also have a
    corresponding STL_IMPLEMENT_SINGLETON statement somewhere in your code.

    @see STL_IMPLEMENT_SINGLETON, STL_DECLARE_SINGLETON
*/
#define STL_DECLARE_SINGLETON_SINGLETHREADED_MINIMAL(Classname) \
\
    static stl::SingletonHolder<Classname, stl::DummyCriticalSection, false> singletonHolder; \
    friend decltype (singletonHolder); \
\
    static Classname* STL_CALLTYPE getInstance()                           { return singletonHolder.getWithoutChecking(); } \
    static Classname* STL_CALLTYPE getInstanceWithoutCreating() noexcept   { return singletonHolder.instance; } \
    static void STL_CALLTYPE deleteInstance() noexcept                     { singletonHolder.deleteInstance(); } \
    void clearSingletonInstance() noexcept                                  { singletonHolder.clear (this); }


//==============================================================================
#ifndef DOXYGEN
 // These are ancient macros, and have now been updated with new names to match the STL style guide,
 // so please update your code to use the newer versions!
 #define stl_DeclareSingleton(Classname, doNotRecreate)                STL_DECLARE_SINGLETON(Classname, doNotRecreate)
 #define stl_DeclareSingleton_SingleThreaded(Classname, doNotRecreate) STL_DECLARE_SINGLETON_SINGLETHREADED(Classname, doNotRecreate)
 #define stl_DeclareSingleton_SingleThreaded_Minimal(Classname)        STL_DECLARE_SINGLETON_SINGLETHREADED_MINIMAL(Classname)
 #define stl_ImplementSingleton(Classname)                             STL_IMPLEMENT_SINGLETON(Classname)
 #define stl_ImplementSingleton_SingleThreaded(Classname)              STL_IMPLEMENT_SINGLETON(Classname)
#endif

} // namespace stl
