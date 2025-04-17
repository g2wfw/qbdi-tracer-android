//
// Created by xb on 2024/10/11.
//

#ifndef MANXI_CORE_STL_EXCEPTION_H
#define MANXI_CORE_STL_EXCEPTION_H

#include <stdexcept>
#include "../stl_macro.h"
#include "../text/stl_String.h"
#include "../logging/formatter.h"

namespace stl {
    class STL_EXPORT Exception : public std::exception {
    public:


        Exception(const std::string &msg, int code = 0);
        /// Creates an exception.

        Exception(const std::string &msg, const std::string &arg, int code = 0);
        /// Creates an exception.

        Exception(const std::string &msg, const Exception &nested, int code = 0);
        /// Creates an exception and stores a clone
        /// of the nested exception.

        Exception(const Exception &exc);
        /// Copy constructor.

        ~Exception() noexcept;
        /// Destroys the exception and deletes the nested exception.

        Exception &operator=(const Exception &exc);
        /// Assignment operator.

        virtual const char *name() const noexcept;
        /// Returns a static string describing the exception.

        virtual const char *className() const noexcept;
        /// Returns the name of the exception class.

        virtual const char *what() const noexcept;
        /// Returns a static string describing the exception.
        ///
        /// Same as name(), but for compatibility with std::exception.

        const Exception *nested() const;
        /// Returns a pointer to the nested exception, or
        /// null if no nested exception exists.

        const String &message() const;
        /// Returns the message text.

        int code() const;
        /// Returns the exception code if defined.

        String displayText() const;
        /// Returns a string consisting of the
        /// message name and the message text.

        virtual Exception *clone() const;
        /// Creates an exact copy of the exception.
        ///
        /// The copy can later be thrown again by
        /// invoking rethrow() on it.

        virtual void rethrow() const;
        /// (Re)Throws the exception.
        ///
        /// This is useful for temporarily storing a
        /// copy of an exception (see clone()), then
        /// throwing it again.

    protected:
        Exception(int code = 0);
        /// Standard constructor.

        void message(const std::string &msg);
        /// Sets the message for the exception.

        void extendedMessage(const std::string &arg);
        /// Sets the extended message for the exception.

    private:
        String _msg;
        Exception *_pNested;
        int _code;
    };


//
// inlines
//
    inline const Exception *Exception::nested() const {
        return _pNested;
    }


    inline const String &Exception::message() const {
        return _msg;
    }


    inline void Exception::message(const std::string &msg) {
        _msg = msg;
    }


    inline int Exception::code() const {
        return _code;
    }






//
// Macros for quickly declaring and implementing exception classes.
// Unfortunately, we cannot use a template here because character
// pointers (which we need for specifying the exception name)
// are not allowed as template arguments.
//
#define STL_DECLARE_EXCEPTION_CODE(API, CLS, BASE, CODE) \
    class API CLS: public BASE                                                        \
    {                                                                                \
    public:                                                                            \
        CLS(int code = CODE);                            \
        CLS(const std::string& msg, int code = CODE);                                \
        CLS(const std::string& msg, const std::string& arg, int code = CODE);        \
        CLS(const std::string& msg, const Exception& exc, int code = CODE);    \
        CLS(const CLS& exc);                                                        \
        ~CLS() noexcept;                                                                \
        CLS& operator = (const CLS& exc);                                            \
        const char* name() const noexcept;                                            \
        const char* className() const noexcept;                                        \
        Exception* clone() const;                                                \
        void rethrow() const;                                                        \
    };

#define STL_DECLARE_EXCEPTION(API, CLS, BASE) \
    STL_DECLARE_EXCEPTION_CODE(API, CLS, BASE, 0)

#define STL_IMPLEMENT_EXCEPTION(CLS, BASE, NAME)                                                    \
    CLS::CLS(int code): BASE(code)                                                                    \
    {                                                                                                \
    }                                                                                                \
    CLS::CLS(const std::string& msg, int code): BASE(msg, code)                                        \
    {                                                                                                \
    }                                                                                                \
    CLS::CLS(const std::string& msg, const std::string& arg, int code): BASE(msg, arg, code)        \
    {                                                                                                \
    }                                                                                                \
    CLS::CLS(const std::string& msg, const Exception& exc, int code): BASE(msg, exc, code)    \
    {                                                                                                \
    }                                                                                                \
    CLS::CLS(const CLS& exc): BASE(exc)                                                                \
    {                                                                                                \
    }                                                                                                \
    CLS::~CLS() noexcept                                                                            \
    {                                                                                                \
    }                                                                                                \
    CLS& CLS::operator = (const CLS& exc)                                                            \
    {                                                                                                \
        BASE::operator = (exc);                                                                        \
        return *this;                                                                                \
    }                                                                                                \
    const char* CLS::name() const noexcept                                                            \
    {                                                                                                \
        return NAME;                                                                                \
    }                                                                                                \
    const char* CLS::className() const noexcept                                                        \
    {                                                                                                \
        return typeid(*this).name();                                                                \
    }                                                                                                \
    Exception* CLS::clone() const                                                                \
    {                                                                                                \
        return new CLS(*this);                                                                        \
    }                                                                                                \
    void CLS::rethrow() const                                                                        \
    {                                                                                                \
        throw *this;                                                                                \
    }


    STL_DECLARE_EXCEPTION(STL_EXPORT, LogicException, Exception)

    STL_DECLARE_EXCEPTION(STL_EXPORT, AssertionViolationException, LogicException)

    STL_DECLARE_EXCEPTION(STL_EXPORT, NullPointerException, LogicException)

    STL_DECLARE_EXCEPTION(STL_EXPORT, NullValueException, LogicException)

    STL_DECLARE_EXCEPTION(STL_EXPORT, BugcheckException, LogicException)

    STL_DECLARE_EXCEPTION(STL_EXPORT, InvalidArgumentException, LogicException)

    STL_DECLARE_EXCEPTION(STL_EXPORT, NotImplementedException, LogicException)

    STL_DECLARE_EXCEPTION(STL_EXPORT, RangeException, LogicException)

    STL_DECLARE_EXCEPTION(STL_EXPORT, IllegalStateException, LogicException)

    STL_DECLARE_EXCEPTION(STL_EXPORT, InvalidAccessException, LogicException)

    STL_DECLARE_EXCEPTION(STL_EXPORT, SignalException, LogicException)

    STL_DECLARE_EXCEPTION(STL_EXPORT, UnhandledException, LogicException)

    STL_DECLARE_EXCEPTION(STL_EXPORT, RuntimeException, Exception)

    STL_DECLARE_EXCEPTION(STL_EXPORT, NotFoundException, RuntimeException)

    STL_DECLARE_EXCEPTION(STL_EXPORT, ExistsException, RuntimeException)

    STL_DECLARE_EXCEPTION(STL_EXPORT, TimeoutException, RuntimeException)

    STL_DECLARE_EXCEPTION(STL_EXPORT, SystemException, RuntimeException)

    STL_DECLARE_EXCEPTION(STL_EXPORT, RegularExpressionException, RuntimeException)

    STL_DECLARE_EXCEPTION(STL_EXPORT, LibraryLoadException, RuntimeException)

    STL_DECLARE_EXCEPTION(STL_EXPORT, LibraryAlreadyLoadedException, RuntimeException)

    STL_DECLARE_EXCEPTION(STL_EXPORT, NoThreadAvailableException, RuntimeException)

    STL_DECLARE_EXCEPTION(STL_EXPORT, PropertyNotSupportedException, RuntimeException)

    STL_DECLARE_EXCEPTION(STL_EXPORT, PoolOverflowException, RuntimeException)

    STL_DECLARE_EXCEPTION(STL_EXPORT, NoPermissionException, RuntimeException)

    STL_DECLARE_EXCEPTION(STL_EXPORT, OutOfMemoryException, RuntimeException)

    STL_DECLARE_EXCEPTION(STL_EXPORT, DataException, RuntimeException)

    STL_DECLARE_EXCEPTION(STL_EXPORT, DataFormatException, DataException)

    STL_DECLARE_EXCEPTION(STL_EXPORT, SyntaxException, DataException)

    STL_DECLARE_EXCEPTION(STL_EXPORT, CircularReferenceException, DataException)

    STL_DECLARE_EXCEPTION(STL_EXPORT, PathSyntaxException, SyntaxException)

    STL_DECLARE_EXCEPTION(STL_EXPORT, IOException, RuntimeException)

    STL_DECLARE_EXCEPTION(STL_EXPORT, ProtocolException, IOException)

    STL_DECLARE_EXCEPTION(STL_EXPORT, FileException, IOException)

    STL_DECLARE_EXCEPTION(STL_EXPORT, FileExistsException, FileException)

    STL_DECLARE_EXCEPTION(STL_EXPORT, FileNotFoundException, FileException)

    STL_DECLARE_EXCEPTION(STL_EXPORT, PathNotFoundException, FileException)

    STL_DECLARE_EXCEPTION(STL_EXPORT, FileReadOnlyException, FileException)

    STL_DECLARE_EXCEPTION(STL_EXPORT, FileAccessDeniedException, FileException)

    STL_DECLARE_EXCEPTION(STL_EXPORT, CreateFileException, FileException)

    STL_DECLARE_EXCEPTION(STL_EXPORT, OpenFileException, FileException)

    STL_DECLARE_EXCEPTION(STL_EXPORT, WriteFileException, FileException)

    STL_DECLARE_EXCEPTION(STL_EXPORT, ReadFileException, FileException)

    STL_DECLARE_EXCEPTION(STL_EXPORT, FileNotReadyException, FileException)

    STL_DECLARE_EXCEPTION(STL_EXPORT, DirectoryNotEmptyException, FileException)

    STL_DECLARE_EXCEPTION(STL_EXPORT, UnknownURISchemeException, RuntimeException)

    STL_DECLARE_EXCEPTION(STL_EXPORT, TooManyURIRedirectsException, RuntimeException)

    STL_DECLARE_EXCEPTION(STL_EXPORT, URISyntaxException, SyntaxException)

    STL_DECLARE_EXCEPTION(STL_EXPORT, ApplicationException, Exception)

    STL_DECLARE_EXCEPTION(STL_EXPORT, BadCastException, RuntimeException)

    STL_DECLARE_EXCEPTION(STL_EXPORT, OutOfRangeException, RuntimeException)

    STL_DECLARE_EXCEPTION(STL_EXPORT, IndexOutOfBoundsException, RuntimeException)
    STL_DECLARE_EXCEPTION(STL_EXPORT, BufferUnderflowException, RuntimeException)


} // stl

#endif //MANXI_CORE_STL_EXCEPTION_H
