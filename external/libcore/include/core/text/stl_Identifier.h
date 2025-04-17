#pragma once

#include "stl_String.h"
#include "stl_StringRef.h"
#include "../containers/stl_Array.h"
namespace stl
{

//==============================================================================
/**
    表示字符串标识符，用于按名称访问属性。
    比较两个Identifier对象非常快（O（1）操作），但是创建
    它们可能比直接使用String慢，因此使用它们的最佳方式
    就是为您经常使用的东西保留一些静态Identifier对象。

    @see NamedValueSet, ValueTree

    @tags{Core}
*/
class STL_EXPORT  Identifier  final
{
public:
    /** Creates a null identifier. */
    Identifier() noexcept;

    /** Creates an identifier with a specified name.
        Because this name may need to be used in contexts such as script variables or XML
        tags, it must only contain ascii letters and digits, or the underscore character.
    */
    Identifier (const char* name);

    /** Creates an identifier with a specified name.
        Because this name may need to be used in contexts such as script variables or XML
        tags, it must only contain ascii letters and digits, or the underscore character.
    */
    Identifier (const String& name);

    /** Creates an identifier with a specified name.
        Because this name may need to be used in contexts such as script variables or XML
        tags, it must only contain ascii letters and digits, or the underscore character.
    */
    Identifier (String::CharPointerType nameStart, String::CharPointerType nameEnd);

    /** Creates a copy of another identifier. */
    Identifier (const Identifier& other) noexcept;

    /** Creates a copy of another identifier. */
    Identifier& operator= (const Identifier& other) noexcept;

    /** Creates a copy of another identifier. */
    Identifier (Identifier&& other) noexcept;

    /** Creates a copy of another identifier. */
    Identifier& operator= (Identifier&& other) noexcept;

    /** Destructor */
    ~Identifier() noexcept;

    /** Compares two identifiers. This is a very fast operation. */
    inline bool operator== (const Identifier& other) const noexcept     { return name.getCharPointer() == other.name.getCharPointer(); }

    /** Compares two identifiers. This is a very fast operation. */
    inline bool operator!= (const Identifier& other) const noexcept     { return name.getCharPointer() != other.name.getCharPointer(); }

    /** Compares the identifier with a string. */
    inline bool operator== (StringRef other) const noexcept             { return name == other; }

    /** Compares the identifier with a string. */
    inline bool operator!= (StringRef other) const noexcept             { return name != other; }

    /** Compares the identifier with a string. */
    inline bool operator<  (StringRef other) const noexcept             { return name <  other; }

    /** Compares the identifier with a string. */
    inline bool operator<= (StringRef other) const noexcept             { return name <= other; }

    /** Compares the identifier with a string. */
    inline bool operator>  (StringRef other) const noexcept             { return name >  other; }

    /** Compares the identifier with a string. */
    inline bool operator>= (StringRef other) const noexcept             { return name >= other; }

    /** Returns this identifier as a string. */
    const String& toString() const noexcept                             { return name; }

    /** Returns this identifier's raw string pointer. */
    operator String::CharPointerType() const noexcept                   { return name.getCharPointer(); }

    /** Returns this identifier's raw string pointer. */
    String::CharPointerType getCharPointer() const noexcept             { return name.getCharPointer(); }

    /** Returns this identifier as a StringRef. */
    operator StringRef() const noexcept                                 { return name; }

    /** Returns true if this Identifier is not null */
    bool isValid() const noexcept                                       { return name.isNotEmpty(); }

    /** Returns true if this Identifier is null */
    bool isNull() const noexcept                                        { return name.isEmpty(); }

    /** A null identifier. */
    static Identifier null;

    /** Checks a given string for characters that might not be valid in an Identifier.
        Since Identifiers are used as a script variables and XML attributes, they should only contain
        alphanumeric characters, underscores, or the '-' and ':' characters.
    */
    static bool isValidIdentifier (const String& possibleIdentifier) noexcept;

private:
    String name;
};

} // namespace stl
