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
#include "stl_core.h"
#include "files/stl_File.h"
#include "files/stl_FileInputStream.h"
#include "files/stl_FileOutputStream.h"
#include "files/stl_RangedDirectoryIterator.h"
#include "threads/stl_Process.h"
#include "files/stl_TemporaryFile.h"
#include "maths/stl_Random.h"
#include "files/stl_MemoryMappedFile.h"
#include "maths/stl_Range.h"
#include "streams/stl_ByteBuffer.h"
#include "files/stl_memory_file.h"

#ifdef STL_MAC
#include <unistd.h>
#endif
namespace stl {

    File::File(const String &fullPathName)
            : fullPath(parseAbsolutePath(fullPathName)) {

    }

    File File::createFileWithoutCheckingPath(const String &path) noexcept {
        File f;
        f.fullPath = path;
        return f;
    }

    File::File(const File &other)
            : fullPath(other.fullPath) {
    }

    File &File::operator=(const String &newPath) {
        fullPath = parseAbsolutePath(newPath);
        return *this;
    }

    File &File::operator=(const File &other) {
        fullPath = other.fullPath;
        return *this;
    }

    File::File(File &&other) noexcept
            : fullPath(std::move(other.fullPath)) {
    }

    File &File::operator=(File &&other) noexcept {
        fullPath = std::move(other.fullPath);
        return *this;
    }

//==============================================================================
    static String removeEllipsis(const String &path) {
        // This will quickly find both /../ and /./ at the expense of a minor
        // false-positive performance hit when path elements end in a dot.
#if STL_WINDOWS
        if (path.contains (".\\"))
#else
        if (path.contains("./"))
#endif
        {
            StringArray toks;
            toks.addTokens(path, File::getSeparatorString(), {});
            bool anythingChanged = false;

            for (int i = 1; i < toks.size(); ++i) {
                auto &t = toks[i];

                if (t == ".." && toks[i - 1] != "..") {
                    anythingChanged = true;
                    toks.removeRange(i - 1, 2);
                    i = jmax(0, i - 2);
                } else if (t == ".") {
                    anythingChanged = true;
                    toks.remove(i--);
                }
            }

            if (anythingChanged)
                return toks.joinIntoString(File::getSeparatorString());
        }

        return path;
    }

    static String normaliseSeparators(const String &path) {
        auto normalisedPath = path;

        String separator(File::getSeparatorString());
        String doubleSeparator(separator + separator);

        auto uncPath = normalisedPath.startsWith(doubleSeparator)
                       && !normalisedPath.fromFirstOccurrenceOf(doubleSeparator, false, false).startsWith(separator);

        if (uncPath)
            normalisedPath = normalisedPath.fromFirstOccurrenceOf(doubleSeparator, false, false);

        while (normalisedPath.contains(doubleSeparator))
            normalisedPath = normalisedPath.replace(doubleSeparator, separator);

        return uncPath ? doubleSeparator + normalisedPath
                       : normalisedPath;
    }

    bool File::isRoot() const {
        return fullPath.isNotEmpty() && *this == getParentDirectory();
    }

    String File::parseAbsolutePath(const String &p) {
        if (p.isEmpty())
            return {};

#if STL_WINDOWS
            // Windows..
            auto path = normaliseSeparators (removeEllipsis (p.replaceCharacter ('/', '\\')));

            if (path.startsWithChar (getSeparatorChar()))
            {
                if (path[1] != getSeparatorChar())
                {
                    /*  When you supply a raw string to the File object constructor, it must be an absolute path.
                        If you're trying to parse a string that may be either a relative path or an absolute path,
                        you MUST provide a context against which the partial path can be evaluated - you can do
                        this by simply using File::getChildFile() instead of the File constructor. E.g. saying
                        "File::getCurrentWorkingDirectory().getChildFile (myUnknownPath)" would return an absolute
                        path if that's what was supplied, or would evaluate a partial path relative to the CWD.
                    */
                    jassertfalse;

                    path = File::getCurrentWorkingDirectory().getFullPathName().substring (0, 2) + path;
                }
            }
            else if (! path.containsChar (':'))
            {
                /*  When you supply a raw string to the File object constructor, it must be an absolute path.
                    If you're trying to parse a string that may be either a relative path or an absolute path,
                    you MUST provide a context against which the partial path can be evaluated - you can do
                    this by simply using File::getChildFile() instead of the File constructor. E.g. saying
                    "File::getCurrentWorkingDirectory().getChildFile (myUnknownPath)" would return an absolute
                    path if that's what was supplied, or would evaluate a partial path relative to the CWD.
                */
                jassertfalse;

                return File::getCurrentWorkingDirectory().getChildFile (path).getFullPathName();
            }
#else
        // Mac or Linux..

        // Yes, I know it's legal for a unix pathname to contain a backslash, but this assertion is here
        // to catch anyone who's trying to run code that was written on Windows with hard-coded path names.
        // If that's why you've ended up here, use File::getChildFile() to build your paths instead.
        jassert((!p.containsChar('\\')) || (p.indexOfChar('/') >= 0 && p.indexOfChar('/') < p.indexOfChar('\\')));

        auto path = normaliseSeparators(removeEllipsis(p));

        if (path.startsWithChar('~')) {
            if (path[1] == getSeparatorChar() || path[1] == 0) {
                // expand a name of the form "~/abc"
                path = File::getSpecialLocation(File::userHomeDirectory).getFullPathName()
                       + path.substring(1);
            } else {
                // expand a name of type "~dave/abc"
                auto userName = path.substring(1).upToFirstOccurrenceOf("/", false, false);

                if (auto *pw = getpwnam(userName.toUTF8()))
                    path = addTrailingSeparator(pw->pw_dir) + path.fromFirstOccurrenceOf("/", false, false);
            }
        } else if (!path.startsWithChar(getSeparatorChar())) {
#if STL_DEBUG || STL_LOG_ASSERTIONS
            if (! (path.startsWith ("./") || path.startsWith ("../")))
            {
                /*  When you supply a raw string to the File object constructor, it must be an absolute path.
                    If you're trying to parse a string that may be either a relative path or an absolute path,
                    you MUST provide a context against which the partial path can be evaluated - you can do
                    this by simply using File::getChildFile() instead of the File constructor. E.g. saying
                    "File::getCurrentWorkingDirectory().getChildFile (myUnknownPath)" would return an absolute
                    path if that's what was supplied, or would evaluate a partial path relative to the CWD.
                */
                jassertfalse;

#if STL_LOG_ASSERTIONS

#endif
            }
#endif

            return File::getCurrentWorkingDirectory().getChildFile(path).getFullPathName();
        }
#endif

        while (path.endsWithChar(getSeparatorChar()) &&
               path != getSeparatorString()) // careful not to turn a single "/" into an empty string.
            path = path.dropLastCharacters(1);

        return path;
    }

    String File::addTrailingSeparator(const String &path) {
        return path.endsWithChar(getSeparatorChar()) ? path
                                                     : path + getSeparatorChar();
    }

//==============================================================================
#if STL_LINUX || STL_BSD
#define NAMES_ARE_CASE_SENSITIVE 1
#endif

    bool File::areFileNamesCaseSensitive() {
#if NAMES_ARE_CASE_SENSITIVE
        return true;
#else
        return false;
#endif
    }

    static int compareFilenames(const String &name1, const String &name2) noexcept {
#if NAMES_ARE_CASE_SENSITIVE
        return name1.compare(name2);
#else
        return name1.compareIgnoreCase(name2);
#endif
    }

    bool File::operator==(const File &other) const { return compareFilenames(fullPath, other.fullPath) == 0; }

    bool File::operator!=(const File &other) const { return compareFilenames(fullPath, other.fullPath) != 0; }

    bool File::operator<(const File &other) const { return compareFilenames(fullPath, other.fullPath) < 0; }

    bool File::operator>(const File &other) const { return compareFilenames(fullPath, other.fullPath) > 0; }

//==============================================================================
    bool File::setReadOnly(const bool shouldBeReadOnly,
                           const bool applyRecursively) const {
        bool worked = true;

        if (applyRecursively && isDirectory())
            for (auto &f: findChildFiles(File::findFilesAndDirectories, false))
                worked = f.setReadOnly(shouldBeReadOnly, true) && worked;

        return setFileReadOnlyInternal(shouldBeReadOnly) && worked;
    }

    bool File::setExecutePermission(bool shouldBeExecutable) const {
        return setFileExecutableInternal(shouldBeExecutable);
    }

    bool File::deleteRecursively(bool followSymlinks) const {
        bool worked = true;

        if (isDirectory() && (followSymlinks || !isSymbolicLink()))
            for (auto &f: findChildFiles(File::findFilesAndDirectories, false))
                worked = f.deleteRecursively(followSymlinks) && worked;

        return deleteFile() && worked;
    }

    bool File::moveFileTo(const File &newFile) const {
        if (newFile.fullPath == fullPath)
            return true;

        if (!exists())
            return false;

#if !NAMES_ARE_CASE_SENSITIVE
        if (*this != newFile)
#endif
        if (!newFile.deleteFile())
            return false;

        return moveInternal(newFile);
    }

    bool File::copyFileTo(const File &newFile) const {
        return (*this == newFile)
               || (exists() && newFile.deleteFile() && copyInternal(newFile));
    }

    bool File::copyFileTo(const String &targetLocation) const {
        File dest(targetLocation);
        if (!dest.getParentDirectory().existsAsDirectory()) {
            dest.getParentDirectory().createDirectory();
        }
        return copyFileTo(dest);
    }

    bool File::replaceFileIn(const File &newFile) const {
        if (newFile.fullPath == fullPath)
            return true;

        if (!newFile.exists())
            return moveFileTo(newFile);

        if (!replaceInternal(newFile))
            return false;

        deleteFile();
        return true;
    }

    bool File::copyDirectoryTo(const File &newDirectory) const {
        if (isDirectory() && newDirectory.createDirectory()) {
            for (auto &f: findChildFiles(File::findFiles, false))
                if (!f.copyFileTo(newDirectory.getChildFile(f.getFileName())))
                    return false;

            for (auto &f: findChildFiles(File::findDirectories, false))
                if (!f.copyDirectoryTo(newDirectory.getChildFile(f.getFileName())))
                    return false;

            return true;
        }

        return false;
    }

//==============================================================================
    String File::getPathUpToLastSlash() const {
        auto lastSlash = fullPath.lastIndexOfChar(getSeparatorChar());

        if (lastSlash > 0)
            return fullPath.substring(0, lastSlash);

        if (lastSlash == 0)
            return getSeparatorString();

        return fullPath;
    }

    File File::getParentDirectory() const {
        return createFileWithoutCheckingPath(getPathUpToLastSlash());
    }

//==============================================================================
    String File::getFileName() const {
        return fullPath.substring(fullPath.lastIndexOfChar(getSeparatorChar()) + 1);
    }

    String File::getFileNameWithoutExtension() const {
        auto lastSlash = fullPath.lastIndexOfChar(getSeparatorChar()) + 1;
        auto lastDot = fullPath.lastIndexOfChar('.');

        if (lastDot > lastSlash)
            return fullPath.substring(lastSlash, lastDot);

        return fullPath.substring(lastSlash);
    }

    bool File::isAChildOf(const File &potentialParent) const {
        if (potentialParent.fullPath.isEmpty())
            return false;

        auto ourPath = getPathUpToLastSlash();

        if (compareFilenames(potentialParent.fullPath, ourPath) == 0)
            return true;

        if (potentialParent.fullPath.length() >= ourPath.length())
            return false;

        return getParentDirectory().isAChildOf(potentialParent);
    }

    int File::hashCode() const { return fullPath.hashCode(); }

    int64 File::hashCode64() const { return fullPath.hashCode64(); }

//==============================================================================
    bool File::isAbsolutePath(StringRef path) {
        auto firstChar = *(path.text);

        return firstChar == getSeparatorChar()
               #if STL_WINDOWS
               || (firstChar != 0 && path.text[1] == ':');
               #else
               || firstChar == '~';
#endif
    }

    File File::getChildFile(StringRef relativePath) const {
        auto r = relativePath.text;

        if (isAbsolutePath(r))
            return File(String(r));

#if STL_WINDOWS
        if (r.indexOf ((stl_wchar) '/') >= 0)
            return getChildFile (String (r).replaceCharacter ('/', '\\'));
#endif

        auto path = fullPath;
        auto separatorChar = getSeparatorChar();

        while (*r == '.') {
            auto lastPos = r;
            auto secondChar = *++r;

            if (secondChar == '.') // remove "../"
            {
                auto thirdChar = *++r;

                if (thirdChar == separatorChar || thirdChar == 0) {
                    auto lastSlash = path.lastIndexOfChar(separatorChar);

                    if (lastSlash >= 0)
                        path = path.substring(0, lastSlash);

                    while (*r == separatorChar) // ignore duplicate slashes
                        ++r;
                } else {
                    r = lastPos;
                    break;
                }
            } else if (secondChar == separatorChar || secondChar == 0)  // remove "./"
            {
                while (*r == separatorChar) // ignore duplicate slashes
                    ++r;
            } else {
                r = lastPos;
                break;
            }
        }

        path = addTrailingSeparator(path);
        path.appendCharPointer(r);
        return File(path);
    }

    File File::getSiblingFile(StringRef fileName) const {
        return getParentDirectory().getChildFile(fileName);
    }

//==============================================================================
    String File::descriptionOfSizeInBytes(const int64 bytes) {
        const char *suffix;
        double divisor = 0;

        if (bytes == 1) { suffix = " byte"; }
        else if (bytes < 1024) { suffix = " bytes"; }
        else if (bytes < 1024 * 1024) {
            suffix = " KB";
            divisor = 1024.0;
        } else if (bytes < 1024 * 1024 * 1024) {
            suffix = " MB";
            divisor = 1024.0 * 1024.0;
        } else {
            suffix = " GB";
            divisor = 1024.0 * 1024.0 * 1024.0;
        }

        return (divisor > 0 ? String((double) bytes / divisor, 1) : String(bytes)) + suffix;
    }

//==============================================================================
    Result File::create() const {
        if (exists())
            return Result::ok();

        auto parentDir = getParentDirectory();

        if (parentDir == *this)
            return Result::fail("Cannot create parent directory");

        auto r = parentDir.createDirectory();

        if (r.wasOk()) {
            FileOutputStream fo(*this, 8);
            r = fo.getStatus();
        }

        return r;
    }

    Result File::createDirectory() const {
        if (isDirectory())
            return Result::ok();

        auto parentDir = getParentDirectory();

        if (parentDir == *this)
            return Result::fail("Cannot create parent directory");

        auto r = parentDir.createDirectory();

        if (r.wasOk())
            r = createDirectoryInternal(fullPath.trimCharactersAtEnd(getSeparatorString()));

        return r;
    }

//==============================================================================
    Time File::getLastModificationTime() const {
        int64 m, a, c;
        getFileTimesInternal(m, a, c);
        return Time(m);
    }

    Time File::getLastAccessTime() const {
        int64 m, a, c;
        getFileTimesInternal(m, a, c);
        return Time(a);
    }

    Time File::getCreationTime() const {
        int64 m, a, c;
        getFileTimesInternal(m, a, c);
        return Time(c);
    }

    bool File::setLastModificationTime(Time t) const { return setFileTimesInternal(t.toMilliseconds(), 0, 0); }

    bool File::setLastAccessTime(Time t) const { return setFileTimesInternal(0, t.toMilliseconds(), 0); }

    bool File::setCreationTime(Time t) const { return setFileTimesInternal(0, 0, t.toMilliseconds()); }

//==============================================================================
    bool File::loadFileAsData(MemoryBlock &destBlock) const {
        if (!existsAsFile())
            return false;

        FileInputStream in(*this);
        return in.openedOk() && getSize() == (int64) in.readIntoMemoryBlock(destBlock);
    }

    String File::loadFileAsString() const {
        if (!existsAsFile())
            return {};

        FileInputStream in(*this);
        return in.openedOk() ? in.readEntireStreamAsString()
                             : String();
    }

    void File::readLines(StringArray &destLines) const {
        destLines.addLines(loadFileAsString());
    }

//==============================================================================
    Array<File> File::findChildFiles(int whatToLookFor, bool searchRecursively, const String &wildcard,
                                     FollowSymlinks followSymlinks) const {
        Array<File> results;
        findChildFiles(results, whatToLookFor, searchRecursively, wildcard, followSymlinks);
        return results;
    }

    int File::findChildFiles(Array<File> &results, int whatToLookFor, bool searchRecursively, const String &wildcard,
                             FollowSymlinks followSymlinks) const {
        int total = 0;

        for (const auto &di: RangedDirectoryIterator(*this, searchRecursively, wildcard, whatToLookFor,
                                                     followSymlinks)) {
            results.add(di.getFile());
            ++total;
        }

        return total;
    }

    int File::getNumberOfChildFiles(const int whatToLookFor, const String &wildCardPattern) const {
        return std::accumulate(RangedDirectoryIterator(*this, false, wildCardPattern, whatToLookFor),
                               RangedDirectoryIterator(),
                               0,
                               [](int acc, const DirectoryEntry &) { return acc + 1; });
    }

    bool File::containsSubDirectories() const {
        if (!isDirectory())
            return false;

        return RangedDirectoryIterator(*this, false, "*", findDirectories) != RangedDirectoryIterator();
    }

//==============================================================================
    File File::getNonexistentChildFile(const String &suggestedPrefix,
                                       const String &suffix,
                                       bool putNumbersInBrackets) const {
        auto f = getChildFile(suggestedPrefix + suffix);

        if (f.exists()) {
            int number = 1;
            auto prefix = suggestedPrefix;

            // remove any bracketed numbers that may already be on the end..
            if (prefix.trim().endsWithChar(')')) {
                putNumbersInBrackets = true;

                auto openBracks = prefix.lastIndexOfChar('(');
                auto closeBracks = prefix.lastIndexOfChar(')');

                if (openBracks > 0
                    && closeBracks > openBracks
                    && prefix.substring(openBracks + 1, closeBracks).containsOnly("0123456789")) {
                    number = prefix.substring(openBracks + 1, closeBracks).getIntValue();
                    prefix = prefix.substring(0, openBracks);
                }
            }

            do {
                auto newName = prefix;

                if (putNumbersInBrackets) {
                    newName << '(' << ++number << ')';
                } else {
                    if (CharacterFunctions::isDigit(prefix.getLastCharacter()))
                        newName << '_'; // pad with an underscore if the name already ends in a digit

                    newName << ++number;
                }

                f = getChildFile(newName + suffix);

            } while (f.exists());
        }

        return f;
    }

    File File::getNonexistentSibling(const bool putNumbersInBrackets) const {
        if (!exists())
            return *this;

        return getParentDirectory().getNonexistentChildFile(getFileNameWithoutExtension(),
                                                            getFileExtension(),
                                                            putNumbersInBrackets);
    }

//==============================================================================
    String File::getFileExtension() const {
        auto indexOfDot = fullPath.lastIndexOfChar('.');

        if (indexOfDot > fullPath.lastIndexOfChar(getSeparatorChar()))
            return fullPath.substring(indexOfDot);

        return {};
    }

    bool File::hasFileExtension(StringRef possibleSuffix) const {
        if (possibleSuffix.isEmpty())
            return fullPath.lastIndexOfChar('.') <= fullPath.lastIndexOfChar(getSeparatorChar());

        auto semicolon = possibleSuffix.text.indexOf((stl_wchar) ';');

        if (semicolon >= 0)
            return hasFileExtension(String(possibleSuffix.text).substring(0, semicolon).trimEnd())
                   || hasFileExtension((possibleSuffix.text + (semicolon + 1)).findEndOfWhitespace());

        if (fullPath.endsWithIgnoreCase(possibleSuffix)) {
            if (possibleSuffix.text[0] == '.')
                return true;

            auto dotPos = fullPath.length() - possibleSuffix.length() - 1;

            if (dotPos >= 0)
                return fullPath[dotPos] == '.';
        }

        return false;
    }

    File File::withFileExtension(StringRef newExtension) const {
        if (fullPath.isEmpty())
            return {};

        auto filePart = getFileName();

        auto lastDot = filePart.lastIndexOfChar('.');

        if (lastDot >= 0)
            filePart = filePart.substring(0, lastDot);

        if (newExtension.isNotEmpty() && newExtension.text[0] != '.')
            filePart << '.';

        return getSiblingFile(filePart + newExtension);
    }

//==============================================================================
    bool File::startAsProcess(const String &parameters) const {
        return exists() && Process::openDocument(fullPath, parameters);
    }

//==============================================================================
    std::unique_ptr<FileInputStream> File::createInputStream() const {
        auto fin = std::make_unique<FileInputStream>(*this);

        if (fin->openedOk())
            return fin;

        return nullptr;
    }

    std::unique_ptr<FileOutputStream> File::createOutputStream(size_t bufferSize) const {
        auto fout = std::make_unique<FileOutputStream>(*this, bufferSize);

        if (fout->openedOk())
            return fout;

        return nullptr;
    }

//==============================================================================
    bool File::appendData(const void *const dataToAppend,
                          const size_t numberOfBytes) const {
        jassert(((ssize_t) numberOfBytes) >= 0);

        if (numberOfBytes == 0)
            return true;

        FileOutputStream fout(*this, 8192);
        return fout.openedOk() && fout.write(dataToAppend, numberOfBytes);
    }

    bool File::replaceWithData(const void *const dataToWrite,
                               const size_t numberOfBytes) const {
        if (numberOfBytes == 0)
            return deleteFile();

        TemporaryFile tempFile(*this, TemporaryFile::useHiddenFile);
        tempFile.getFile().appendData(dataToWrite, numberOfBytes);
        return tempFile.overwriteTargetFileWithTemporary();
    }

    bool File::appendText(const String &text, bool asUnicode, bool writeHeaderBytes, const char *lineFeed) const {
        FileOutputStream fout(*this);

        if (fout.failedToOpen())
            return false;

        return fout.writeText(text, asUnicode, writeHeaderBytes, lineFeed);
    }

    bool File::replaceWithText(const String &textToWrite, bool asUnicode, bool writeHeaderBytes,
                               const char *lineFeed) const {
        TemporaryFile tempFile(*this, TemporaryFile::useHiddenFile);
        tempFile.getFile().appendText(textToWrite, asUnicode, writeHeaderBytes, lineFeed);
        return tempFile.overwriteTargetFileWithTemporary();
    }

    bool File::hasIdenticalContentTo(const File &other) const {
        if (other == *this)
            return true;

        if (getSize() == other.getSize() && existsAsFile() && other.existsAsFile()) {
            FileInputStream in1(*this), in2(other);

            if (in1.openedOk() && in2.openedOk()) {
                const int bufferSize = 4096;
                HeapBlock<char> buffer1(bufferSize), buffer2(bufferSize);

                for (;;) {
                    auto num1 = in1.read(buffer1, bufferSize);
                    auto num2 = in2.read(buffer2, bufferSize);

                    if (num1 != num2)
                        break;

                    if (num1 <= 0)
                        return true;

                    if (memcmp(buffer1, buffer2, (size_t) num1) != 0)
                        break;
                }
            }
        }

        return false;
    }

//==============================================================================
    String File::createLegalPathName(const String &original) {
        auto s = original;
        String start;

        if (s.isNotEmpty() && s[1] == ':') {
            start = s.substring(0, 2);
            s = s.substring(2);
        }

        return start + s.removeCharacters("\"#@,;:<>*^|?")
                .substring(0, 1024);
    }

    String File::createLegalFileName(const String &original) {
        auto s = original.removeCharacters("\"#@,;:<>*^|?\\/");

        const int maxLength = 128; // only the length of the filename, not the whole path
        auto len = s.length();

        if (len > maxLength) {
            auto lastDot = s.lastIndexOfChar('.');

            if (lastDot > jmax(0, len - 12)) {
                s = s.substring(0, maxLength - (len - lastDot))
                    + s.substring(lastDot);
            } else {
                s = s.substring(0, maxLength);
            }
        }

        return s;
    }

//==============================================================================
    static int countNumberOfSeparators(String::CharPointerType s) {
        int num = 0;

        for (;;) {
            auto c = s.getAndAdvance();

            if (c == 0)
                break;

            if (c == File::getSeparatorChar())
                ++num;
        }

        return num;
    }

    String File::getRelativePathFrom(const File &dir) const {
        if (dir == *this)
            return ".";

        auto thisPath = fullPath;

        while (thisPath.endsWithChar(getSeparatorChar()))
            thisPath = thisPath.dropLastCharacters(1);

        auto dirPath = addTrailingSeparator(dir.existsAsFile() ? dir.getParentDirectory().getFullPathName()
                                                               : dir.fullPath);

        int commonBitLength = 0;
        auto thisPathAfterCommon = thisPath.getCharPointer();
        auto dirPathAfterCommon = dirPath.getCharPointer();

        {
            auto thisPathIter = thisPath.getCharPointer();
            auto dirPathIter = dirPath.getCharPointer();

            for (int i = 0;;) {
                auto c1 = thisPathIter.getAndAdvance();
                auto c2 = dirPathIter.getAndAdvance();

#if NAMES_ARE_CASE_SENSITIVE
                if (c1 != c2
                    #else
                    if ((c1 != c2 && CharacterFunctions::toLowerCase(c1) != CharacterFunctions::toLowerCase(c2))
                    #endif
                    || c1 == 0)
                    break;

                ++i;

                if (c1 == getSeparatorChar()) {
                    thisPathAfterCommon = thisPathIter;
                    dirPathAfterCommon = dirPathIter;
                    commonBitLength = i;
                }
            }
        }

        // if the only common bit is the root, then just return the full path..
        if (commonBitLength == 0 || (commonBitLength == 1 && thisPath[1] == getSeparatorChar()))
            return fullPath;

        auto numUpDirectoriesNeeded = countNumberOfSeparators(dirPathAfterCommon);

        if (numUpDirectoriesNeeded == 0)
            return thisPathAfterCommon;

#if STL_WINDOWS
        auto s = String::repeatedString ("..\\", numUpDirectoriesNeeded);
#else
        auto s = String::repeatedString("../", numUpDirectoriesNeeded);
#endif
        s.appendCharPointer(thisPathAfterCommon);
        return s;
    }

//==============================================================================
    File File::createTempFile(StringRef fileNameEnding) {
        auto tempFile = getSpecialLocation(tempDirectory)
                .getChildFile("temp_" + String::toHexString(Random::getSystemRandom().nextInt()))
                .withFileExtension(fileNameEnding);

        if (tempFile.exists())
            return createTempFile(fileNameEnding);

        return tempFile;
    }

    bool File::createSymbolicLink(const File &linkFileToCreate,
                                  [[maybe_unused]] const String &nativePathOfTarget,
                                  bool overwriteExisting) {
        if (linkFileToCreate.exists()) {
            if (!linkFileToCreate.isSymbolicLink()) {
                // user has specified an existing file / directory as the link
                // this is bad! the user could end up unintentionally destroying data
                jassertfalse;
                return false;
            }

            if (overwriteExisting)
                linkFileToCreate.deleteFile();
        }

#if STL_MAC || STL_LINUX || STL_BSD
        // one common reason for getting an error here is that the file already exists
        if (symlink(nativePathOfTarget.toRawUTF8(), linkFileToCreate.getFullPathName().toRawUTF8()) == -1) {
            jassertfalse;
            return false;
        }

        return true;
#elif STL_MSVC
        File targetFile (linkFileToCreate.getSiblingFile (nativePathOfTarget));

        return CreateSymbolicLink (linkFileToCreate.getFullPathName().toWideCharPointer(),
                                   nativePathOfTarget.toWideCharPointer(),
                                   targetFile.isDirectory() ? SYMBOLIC_LINK_FLAG_DIRECTORY : 0) != FALSE;
#else
        jassertfalse; // symbolic links not supported on this platform!
        return false;
#endif
    }

    bool File::createSymbolicLink(const File &linkFileToCreate, bool overwriteExisting) const {
        return createSymbolicLink(linkFileToCreate, getFullPathName(), overwriteExisting);
    }

    String File::fileName(const String &pathName) {
        return File(pathName).getFileName();
    }

    String File::readToString() const {
        auto stream = this->createInputStream();
        if (stream == nullptr) {
            return "";
        }
        return stream->readEntireStreamAsString();
    }

    bool File::exists(const String &path) {
        return File(path).exists();
    }

    bool File::existsAsFile(const String &path) {
        return File(path).existsAsFile();
    }

    bool File::existsAsDirectory(const String &path) {
        return File(path).existsAsDirectory();
    }

    bool File::isDirectory(const String &path) {
        return File(path).isDirectory();
    }

    std::unique_ptr<ByteBuffer> File::readByteBuffer() const {
        if (!existsAsFile()) {
            return nullptr;
        }
        if (getSize() <= 0) {
            return std::make_unique<ByteBuffer>(createInputStream().get());
        }
        return std::make_unique<ByteBuffer>((File *) this, true);
    }

    std::string File::readToSTDString() const {
        auto buffer = readByteBuffer();
        if (buffer == nullptr) {
            return {};
        }
        std::string data = std::string((const char *) buffer->getRawBuffer(), buffer->size());
        return data;
    }


    std::unique_ptr<MemoryFile> File::toMemoryFile(bool readOnly) const {
        if (!existsAsFile()) {
            return nullptr;
        }
        auto path = getFullPathName();
        return std::make_unique<MemoryFile>(path.c_str(), readOnly);
    }


#if !STL_WINDOWS

    File File::getLinkedTarget() const {
        if (isSymbolicLink())
            return getSiblingFile(getNativeLinkedTarget());

        return *this;
    }

#else
    bool File::existsAsDirectory() const {
        return exists() && isDirectory();
    }



#endif

//==============================================================================
#if STL_ALLOW_STATIC_NULL_VARIABLES

    STL_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wdeprecated-declarations")
    STL_BEGIN_IGNORE_WARNINGS_MSVC (4996)

    const File File::nonexistent{};

    STL_END_IGNORE_WARNINGS_GCC_LIKE
    STL_END_IGNORE_WARNINGS_MSVC

#endif

//==============================================================================
    MemoryMappedFile::MemoryMappedFile(const File &file, MemoryMappedFile::AccessMode mode, bool exclusive)
            : range(0, file.getSize()) {
        openInternal(file, mode, exclusive);
    }

    MemoryMappedFile::MemoryMappedFile(const File &file, const Range<int64> &fileRange, AccessMode mode,
                                       bool exclusive)
            : range(fileRange.getIntersectionWith(Range<int64>(0, file.getSize()))) {
        openInternal(file, mode, exclusive);
    }



} // namespace stl
