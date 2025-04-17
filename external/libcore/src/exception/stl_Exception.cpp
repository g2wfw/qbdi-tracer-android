//
// Created by xb on 2024/10/11.
//

#include "exception/stl_Exception.h"
#include "logging/logger.h"
#include "logging/stllog.h"

namespace stl {

    Exception::Exception(int code) : _pNested(0), _code(code) {

    }


    Exception::Exception(const std::string &msg, int code) : _msg(msg), _pNested(0), _code(code) {
    }


    Exception::Exception(const std::string &msg, const std::string &arg, int code) : _msg(msg), _pNested(0),
                                                                                     _code(code) {
        if (!arg.empty()) {
            _msg.append(": ");
            _msg.append(arg);
        }
    }


    Exception::Exception(const std::string &msg, const Exception &nested, int code) : _msg(msg),
                                                                                      _pNested(nested.clone()),
                                                                                      _code(code) {
    }


    Exception::Exception(const Exception &exc) :
            std::exception(exc),
            _msg(exc._msg),
            _code(exc._code) {
        _pNested = exc._pNested ? exc._pNested->clone() : 0;
    }


    Exception::~Exception() noexcept {
        delete _pNested;
    }


    Exception &Exception::operator=(const Exception &exc) {
        if (&exc != this) {
            Exception *newPNested = exc._pNested ? exc._pNested->clone() : 0;
            delete _pNested;
            _msg = exc._msg;
            _pNested = newPNested;
            _code = exc._code;
        }
        return *this;
    }


    const char *Exception::name() const noexcept {
        return "Exception";
    }


    const char *Exception::className() const noexcept {
        return typeid(*this).name();
    }


    const char *Exception::what() const noexcept {
        return name();
    }


    stl::String Exception::displayText() const {
        String txt = name();
        if (!_msg.isEmpty()) {
            txt.append(": ");
            txt.append(_msg);
        }
        return txt;
    }


    void Exception::extendedMessage(const std::string &arg) {
        if (!arg.empty()) {
            if (!_msg.isEmpty()) _msg.append(": ");
            _msg.append(arg);
        }
    }


    Exception *Exception::clone() const {
        return new Exception(*this);
    }


    void Exception::rethrow() const {
        throw *this;
    }
    STL_IMPLEMENT_EXCEPTION(LogicException, Exception, "Logic exception")

    STL_IMPLEMENT_EXCEPTION(AssertionViolationException, LogicException, "Assertion violation")

    STL_IMPLEMENT_EXCEPTION(NullPointerException, LogicException, "Null pointer")

    STL_IMPLEMENT_EXCEPTION(NullValueException, LogicException, "Null value")

    STL_IMPLEMENT_EXCEPTION(BugcheckException, LogicException, "Bugcheck")

    STL_IMPLEMENT_EXCEPTION(InvalidArgumentException, LogicException, "Invalid argument")

    STL_IMPLEMENT_EXCEPTION(NotImplementedException, LogicException, "Not implemented")

    STL_IMPLEMENT_EXCEPTION(RangeException, LogicException, "Out of range")

    STL_IMPLEMENT_EXCEPTION(IllegalStateException, LogicException, "Illegal state")

    STL_IMPLEMENT_EXCEPTION(InvalidAccessException, LogicException, "Invalid access")

    STL_IMPLEMENT_EXCEPTION(SignalException, LogicException, "Signal received")

    STL_IMPLEMENT_EXCEPTION(UnhandledException, LogicException, "Unhandled exception")

    STL_IMPLEMENT_EXCEPTION(RuntimeException, Exception, "Runtime exception")

    STL_IMPLEMENT_EXCEPTION(NotFoundException, RuntimeException, "Not found")

    STL_IMPLEMENT_EXCEPTION(ExistsException, RuntimeException, "Exists")

    STL_IMPLEMENT_EXCEPTION(TimeoutException, RuntimeException, "Timeout")

    STL_IMPLEMENT_EXCEPTION(SystemException, RuntimeException, "System exception")

    STL_IMPLEMENT_EXCEPTION(RegularExpressionException, RuntimeException, "Error in regular expression")

    STL_IMPLEMENT_EXCEPTION(LibraryLoadException, RuntimeException, "Cannot load library")

    STL_IMPLEMENT_EXCEPTION(LibraryAlreadyLoadedException, RuntimeException, "Library already loaded")

    STL_IMPLEMENT_EXCEPTION(NoThreadAvailableException, RuntimeException, "No thread available")

    STL_IMPLEMENT_EXCEPTION(PropertyNotSupportedException, RuntimeException, "Property not supported")

    STL_IMPLEMENT_EXCEPTION(PoolOverflowException, RuntimeException, "Pool overflow")

    STL_IMPLEMENT_EXCEPTION(NoPermissionException, RuntimeException, "No permission")

    STL_IMPLEMENT_EXCEPTION(OutOfMemoryException, RuntimeException, "Out of memory")

    STL_IMPLEMENT_EXCEPTION(DataException, RuntimeException, "Data error")

    STL_IMPLEMENT_EXCEPTION(DataFormatException, DataException, "Bad data format")

    STL_IMPLEMENT_EXCEPTION(SyntaxException, DataException, "Syntax error")

    STL_IMPLEMENT_EXCEPTION(CircularReferenceException, DataException, "Circular reference")

    STL_IMPLEMENT_EXCEPTION(PathSyntaxException, SyntaxException, "Bad path syntax")

    STL_IMPLEMENT_EXCEPTION(IOException, RuntimeException, "I/O error")

    STL_IMPLEMENT_EXCEPTION(ProtocolException, IOException, "Protocol error")

    STL_IMPLEMENT_EXCEPTION(FileException, IOException, "File access error")

    STL_IMPLEMENT_EXCEPTION(FileExistsException, FileException, "File exists")

    STL_IMPLEMENT_EXCEPTION(FileNotFoundException, FileException, "File not found")

    STL_IMPLEMENT_EXCEPTION(PathNotFoundException, FileException, "Path not found")

    STL_IMPLEMENT_EXCEPTION(FileReadOnlyException, FileException, "File is read-only")

    STL_IMPLEMENT_EXCEPTION(FileAccessDeniedException, FileException, "Access to file denied")

    STL_IMPLEMENT_EXCEPTION(CreateFileException, FileException, "Cannot create file")

    STL_IMPLEMENT_EXCEPTION(OpenFileException, FileException, "Cannot open file")

    STL_IMPLEMENT_EXCEPTION(WriteFileException, FileException, "Cannot write file")

    STL_IMPLEMENT_EXCEPTION(ReadFileException, FileException, "Cannot read file")

    STL_IMPLEMENT_EXCEPTION(FileNotReadyException, FileException, "File not ready")

    STL_IMPLEMENT_EXCEPTION(DirectoryNotEmptyException, FileException, "Directory not empty")

    STL_IMPLEMENT_EXCEPTION(UnknownURISchemeException, RuntimeException, "Unknown URI scheme")

    STL_IMPLEMENT_EXCEPTION(TooManyURIRedirectsException, RuntimeException, "Too many URI redirects")

    STL_IMPLEMENT_EXCEPTION(URISyntaxException, SyntaxException, "Bad URI syntax")

    STL_IMPLEMENT_EXCEPTION(ApplicationException, Exception, "Application exception")

    STL_IMPLEMENT_EXCEPTION(BadCastException, RuntimeException, "Bad cast exception")

    STL_IMPLEMENT_EXCEPTION(OutOfRangeException, RuntimeException, "index is out of range")

    STL_IMPLEMENT_EXCEPTION(IndexOutOfBoundsException, RuntimeException, "index out of bounds")
    STL_IMPLEMENT_EXCEPTION(BufferUnderflowException, RuntimeException, "buffer underflow");

} // stl