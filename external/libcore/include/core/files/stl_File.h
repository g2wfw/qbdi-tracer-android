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
#include "../text/stl_StringArray.h"
#include "../time/stl_Time.h"
#include "../misc/stl_Result.h"
#include "../containers/stl_Array.h"


namespace stl {
    class ByteBuffer;

    class MemoryFile;

#if !DOXYGEN && (STL_MAC || STL_IOS)
    using OSType = unsigned int;
#endif


    class STL_API File final {
    public:
        //==============================================================================
        /** 创建一个（无效）文件对象。
            文件最初设置为空路径，因此getFullPathName（）将返回
            空字符串。
            您可以使用它的operator=方法将其指向正确的文件。
        */
        File() = default;

        /** 从绝对路径创建文件。
        如果提供的路径是相对路径，则将其视为相对路径
        到当前工作目录（请参见文件：：getCurrentWorkingDirectory（）），
        但这不是创建文件的推荐方法，因为
        永远不知道CWD会是什么。
        在Mac/Linux上，路径可以包括“~”符号，用于引用
        用户主目录。
        */
        File(const String &absolutePath);

        /** Creates a copy of another file object. */
        File(const File &);

        /** Destructor. */
        ~File() = default;

        /** 基于绝对路径名设置文件。
            如果提供的路径是相对路径，则将其视为相对路径
            到当前工作目录（请参见文件：：getCurrentWorkingDirectory（）），
            但这不是创建文件的推荐方法，因为
            永远不知道CWD会是什么。
            在Mac/Linux上，路径可以包括“~”符号，用于引用
            用户主目录。
        */
        File &operator=(const String &newAbsolutePath);

        /** Copies from another file object. */
        File &operator=(const File &otherFile);

        /** Move constructor */
        File(File &&) noexcept;

        /** Move assignment operator */
        File &operator=(File &&) noexcept;

        //==============================================================================
        /** Checks whether the file actually exists.

            @returns    true if the file exists, either as a file or a directory.
            @see existsAsFile, isDirectory
        */
        bool exists() const;

        static bool exists(const String &path);


        /** 检查该文件是否存在，是否为文件而非目录。
            @仅当这是真实文件时返回true，如果是目录则返回false
            或不存在
            @请参阅存在，isDirectory
        */
        bool existsAsFile() const;

        /**
         *  检查该文件是否存在，是否为文件而非目录。这个是个静态函数
         * @param path
         * @return
         */
        static bool existsAsFile(const String &path);

        /**
         * 判断文件是否以文件夹的形式存在
         * @return
         */
        bool existsAsDirectory() const;

        /**
         * 静态函数，判断目标信息是否以文件夹的形式存在
         * @param path
         * @return
         */
        static bool existsAsDirectory(const String &path);

        /** 检查文件是否为已存在的目录。
            @仅当文件是实际存在的目录时才返回true，因此
            如果是文件或根本不存在，则为false
        */
        bool isDirectory() const;

        static bool isDirectory(const String &path);


        /**检查此文件的路径是否表示文件系统的根，
        不管它的存在。
        对于Windows上的“C:”、“D:”等和其他系统上的“/”，这将返回true
        平台。
        */
        bool isRoot() const;

        /** Returns the size of the file in bytes.

            @returns    the number of bytes in the file, or 0 if it doesn't exist.
        */
        int64 getSize() const;

        /** 实用程序函数，用于将以字节为单位的文件大小转换为整洁的字符串描述。
            例如，100将返回“100字节”，2000将返回“2KB”，
            2000000将产生“2MB”等。
        */
        static String descriptionOfSizeInBytes(int64 bytes);

        //==============================================================================
        /** 返回此文件的完整绝对路径。
            这包括文件名及其所有父文件夹。在Windows上
            还包括驱动器号前缀；在Mac或Linux上，它将是一个完整的
            从根文件夹开始的路径。
            如果您只想要文件名，则应该使用getFileName（）或
            getFileNameWithoutExtension（）。
            @see getFileName, getRelativePathFrom
        */
        const String &getFullPathName() const noexcept { return fullPath; }

        /** 返回路径名的最后一部分。
            只返回路径的最后一部分-例如，如果整个路径
            是“/mouse/fish/foo.txt”，这将返回“foo.txt”。
            对于目录，它返回路径的最后一部分，例如
            目录“/mouse/fish”，它将返回“fish”。
            如果文件名以句点开头，它将返回整个文件名，例如
            “/mouse/.fish”，它会返回“.fish”
            @see getFullPathName, getFileNameWithoutExtension
        */
        String getFileName() const;

        /** 创建相对于给定目录引用文件的相对路径。
            例如File（“/mouse/foo.txt”）.getRelativePathFrom（File（“/meouse/fish/haddock”）
            将返回“../../foo.txt”。
            如果无法从一个文件导航到另一个文件，则绝对
            返回路径。如果路径无效，空字符串也可能是
            返回。

            @param directoryToBeRelativeTo  the directory which the resultant string will
                                            be relative to. If this is actually a file rather than
                                            a directory, its parent directory will be used instead.
                                            If it doesn't exist, it's assumed to be a directory.
            @see getChildFile, isAbsolutePath
        */
        String getRelativePathFrom(const File &directoryToBeRelativeTo) const;

        //==============================================================================
        /** 返回文件的扩展名。
            返回此文件的文件扩展名，还包括句点。
            例如“/mouse/fish/foo.txt”将返回“.txt”

            @see hasFileExtension, withFileExtension, getFileNameWithoutExtension
        */
        String getFileExtension() const;

        /** Checks whether the file has a given extension.

            @param extensionToTest  the extension to look for - it doesn't matter whether or
                                    not this string has a dot at the start, so ".wav" and "wav"
                                    will have the same effect. To compare with multiple extensions, this
                                    parameter can contain multiple strings, separated by semi-colons -
                                    so, for example: hasFileExtension (".jpeg;png;gif") would return
                                    true if the file has any of those three extensions.

            @see getFileExtension, withFileExtension, getFileNameWithoutExtension
        */
        bool hasFileExtension(StringRef extensionToTest) const;

        /** 返回具有不同文件扩展名的此文件的版本。
            例如File（“/mouse/fish/foo.txt”）。withFileExtension（“html”）返回“/mousE/fish/foo.html”

            @param newExtension    新的扩展名，在开头有点或没有点（这个
                                    没有任何区别）。为了完全删除文件的扩展名，
                                    将一个空字符串传递到此函数中。

            @see getFileName, getFileExtension, hasFileExtension, getFileNameWithoutExtension
        */
        File withFileExtension(StringRef newExtension) const;

        /** Returns the last part of the filename, without its file extension.

            e.g. for "/moose/fish/foo.txt" this will return "foo".

            @see getFileName, getFileExtension, hasFileExtension, withFileExtension
        */
        String getFileNameWithoutExtension() const;

        //==============================================================================
        /** Returns a 32-bit hash-code that identifies this file.

            This is based on the filename. Obviously it's possible, although unlikely, that
            two files will have the same hash-code.
        */
        int hashCode() const;

        /** Returns a 64-bit hash-code that identifies this file.

            This is based on the filename. Obviously it's possible, although unlikely, that
            two files will have the same hash-code.
        */
        int64 hashCode64() const;

        String readToString() const;

        //deprecated
        std::string readToSTDString() const;

        inline std::string readToSTDStringV2() const {
            return this->readToString().string();
        }
        //==============================================================================
        /** 返回表示当前子路径的相对（或绝对）子路径的文件。
            这将查找当前对象的子文件或目录。
            例如。
            文件（“/moose/fish”）.getChildFile（“foo.txt”）将生成“/mouse/fish/foo.txt”。
            文件（“/moose/fish”）.getChildFile（“haddock/foo.txt”）将生成“/mouse/fish/haddock/footxt”。
            文件（“/mouse/fish”）.getChildFile（“../foo.txt”）将生成“/mous/foo.txt”。
            如果字符串实际上是一个绝对路径，它将被视为绝对路径，例如。
            文件（“/mouse/fish”）.getChildFile（“/foo.txt”）将生成“/foo.txt”

            @see getSiblingFile, getParentDirectory, getRelativePathFrom, isAChildOf
        */
        File getChildFile(StringRef relativeOrAbsolutePath) const;

        /**返回与此文件位于同一目录中的文件。
            这相当于getParentDirectory（）.getChildFile（name）。

            @see getChildFile, getParentDirectory
        */
        File getSiblingFile(StringRef siblingFileName) const;

        //==============================================================================
        /** 返回包含此文件或目录的目录。
            例如，对于“/mouse/fish/foo.txt”，它将返回“/mousE/fish”。
            如果您已经在根目录（“/”或“C:”）中，则此方法将
            返回根目录。
        */
        File getParentDirectory() const;

        /** 检查文件是否位于目录中的某个位置。
            如果此文件位于目录的子目录中，则返回true
            这两个文件实际上都不必存在，因为函数
            只是检查路径是否相似。

            e.g. File ("/moose/fish/foo.txt").isAChildOf ("/moose") is true.
                 File ("/moose/fish/foo.txt").isAChildOf ("/moose/fish") is also true.
        */
        bool isAChildOf(const File &potentialParentDirectory) const;

        //==============================================================================
        /** Chooses a filename relative to this one that doesn't already exist.

            If this file is a directory, this will return a child file of this
            directory that doesn't exist, by adding numbers to a prefix and suffix until
            it finds one that isn't already there.

            If the prefix + the suffix doesn't exist, it won't bother adding a number.

            e.g. File ("/moose/fish").getNonexistentChildFile ("foo", ".txt", true) might
                 return "/moose/fish/foo(2).txt" if there's already a file called "foo.txt".

            @param prefix                   the string to use for the filename before the number
            @param suffix                   the string to add to the filename after the number
            @param putNumbersInBrackets     if true, this will create filenames in the
                                            format "prefix(number)suffix", if false, it will leave the
                                            brackets out.
        */
        File getNonexistentChildFile(const String &prefix,
                                     const String &suffix,
                                     bool putNumbersInBrackets = true) const;

        /** Chooses a filename for a sibling file to this one that doesn't already exist.

            If this file doesn't exist, this will just return itself, otherwise it
            will return an appropriate sibling that doesn't exist, e.g. if a file
            "/moose/fish/foo.txt" exists, this might return "/moose/fish/foo(2).txt".

            @param putNumbersInBrackets     whether to add brackets around the numbers that
                                            get appended to the new filename.
        */
        File getNonexistentSibling(bool putNumbersInBrackets = true) const;

        //==============================================================================
        /** Compares the pathnames for two files. */
        bool operator==(const File &) const;

        /** Compares the pathnames for two files. */
        bool operator!=(const File &) const;

        /** Compares the pathnames for two files. */
        bool operator<(const File &) const;

        /** Compares the pathnames for two files. */
        bool operator>(const File &) const;

        //==============================================================================
        /** Checks whether a file can be created or written to.

            @returns    true if it's possible to create and write to this file. If the file
                        doesn't already exist, this will check its parent directory to
                        see if writing is allowed.
            @see setReadOnly
        */
        bool hasWriteAccess() const;

        /** Checks whether a file can be read.

            @returns    true if it's possible to read this file.
        */
        bool hasReadAccess() const;

        /** Changes the write-permission of a file or directory.

            @param shouldBeReadOnly     whether to add or remove write-permission
            @param applyRecursively     if the file is a directory and this is true, it will
                                        recurse through all the subfolders changing the permissions
                                        of all files
            @returns    true if it manages to change the file's permissions.
            @see hasWriteAccess
        */
        bool setReadOnly(bool shouldBeReadOnly,
                         bool applyRecursively = false) const;

        /** Changes the execute-permissions of a file.

            @param shouldBeExecutable   whether to add or remove execute-permission
            @returns    true if it manages to change the file's permissions.
        */
        bool setExecutePermission(bool shouldBeExecutable) const;

        /** Returns true if this file is a hidden or system file.
            The criteria for deciding whether a file is hidden are platform-dependent.
        */
        bool isHidden() const;

        /** Returns a unique identifier for the file, if one is available.

            Depending on the OS and file-system, this may be a unix inode number or
            a win32 file identifier, or 0 if it fails to find one. The number will
            be unique on the filesystem, but not globally.
        */
        uint64 getFileIdentifier() const;

        //==============================================================================
        /** Returns the last modification time of this file.

            @returns    the time, or the Unix Epoch if the file doesn't exist.
            @see setLastModificationTime, getLastAccessTime, getCreationTime
        */
        Time getLastModificationTime() const;

        /** Returns the last time this file was accessed.

            @returns    the time, or the Unix Epoch if the file doesn't exist.
            @see setLastAccessTime, getLastModificationTime, getCreationTime
        */
        Time getLastAccessTime() const;

        /** Returns the time that this file was created.

            @returns    the time, or the Unix Epoch if the file doesn't exist.
            @see getLastModificationTime, getLastAccessTime
        */
        Time getCreationTime() const;

        /** Changes the modification time for this file.

            @param newTime  the time to apply to the file
            @returns true if it manages to change the file's time.
            @see getLastModificationTime, setLastAccessTime, setCreationTime
        */
        bool setLastModificationTime(Time newTime) const;

        /** Changes the last-access time for this file.

            @param newTime  the time to apply to the file
            @returns true if it manages to change the file's time.
            @see getLastAccessTime, setLastModificationTime, setCreationTime
        */
        bool setLastAccessTime(Time newTime) const;

        /** Changes the creation date for this file.

            @param newTime  the time to apply to the file
            @returns true if it manages to change the file's time.
            @see getCreationTime, setLastModificationTime, setLastAccessTime
        */
        bool setCreationTime(Time newTime) const;

        /** 如果可能，这将尝试为给定文件创建一个版本字符串。
            操作系统可能能够查看文件并给出其版本，例如
            可执行文件、捆绑包、dll等。如果没有可用的版本，这将
            返回一个空字符串。
        */
        String getVersion() const;

        //==============================================================================
        /** C如果不存在，则创建一个空文件。
        如果此对象引用的文件不存在，则会创建一个文件
        零大小。
        如果它已经存在或是一个目录，则此方法将不起任何作用。
        如果文件的父目录不存在，则此方法将
        递归地创建父目录。

            @returns   指示文件是否成功创建的结果，如果失败，则显示错误消息。
            @see createDirectory
        */
        Result create() const;

        /** 为此文件名创建一个新目录。
            这将尝试将文件创建为目录，并且还会创建
            完成操作所需的任何父目录。

            @returns    指示目录是否成功创建的结果，或者
                        如果失败，则显示错误消息。
            @see create
        */
        Result createDirectory() const;

        /** 删除文件。
            如果此文件实际上是一个目录，则如果
            包含文件。请参阅deleteRecursively（）作为删除目录的更好方法。
            如果此文件是符号链接，则符号链接将被删除，而不是目标
            符号链接的。

            @returns    true if the file has been successfully deleted (or if it didn't exist to
                        begin with).
            @see deleteRecursively
        */
        bool deleteFile() const;

        /** Deletes a file or directory and all its subdirectories.

            If this file is a directory, this will try to delete it and all its subfolders. If
            it's just a file, it will just try to delete the file.


            @param followSymlinks If true, then any symlink pointing to a directory will also
                                  recursively delete the contents of that directory
            @returns              true if the file and all its subfolders have been successfully
                                  deleted (or if it didn't exist to begin with).
            @see deleteFile
        */
        bool deleteRecursively(bool followSymlinks = false) const;

        /** Moves this file or folder to the trash.

            @returns true if the operation succeeded. It could fail if the trash is full, or
                     if the file is write-protected, so you should check the return value
                     and act appropriately.
        */
        bool moveToTrash() const;

        /** Moves or renames a file.

            Tries to move a file to a different location.
            If the target file already exists, this will attempt to delete it first, and
            will fail if this can't be done.

            Note that the destination file isn't the directory to put it in, it's the actual
            filename that you want the new file to have.

            Also note that on some OSes (e.g. Windows), moving files between different
            volumes may not be possible.

            @returns    true if the operation succeeds
        */
        bool moveFileTo(const File &targetLocation) const;

        /** Copies a file.

            Tries to copy a file to a different location. If the target file already exists,
            this will attempt to delete it first, and will fail if this can't be done.

            Note that the target file isn't the directory to put it in, it's the actual
            filename that you want the new file to have.

            @returns    true if the operation succeeds
        */
        bool copyFileTo(const File &targetLocation) const;

        bool copyFileTo(const String &targetLocation) const;

        /** Replaces a file.

            Replace the file in the given location, assuming the replaced files identity.
            Depending on the file system this will preserve file attributes such as
            creation date, short file name, etc.

            If replacement succeeds the original file is deleted.

            @returns    true if the operation succeeds
        */
        bool replaceFileIn(const File &targetLocation) const;

        /** Copies a directory.

            Tries to copy an entire directory, recursively.

            If this file isn't a directory or if any target files can't be created, this
            will return false.

            @param newDirectory    the directory that this one should be copied to. Note that this
                                   is the name of the actual directory to create, not the directory
                                   into which the new one should be placed, so there must be enough
                                   write privileges to create it if it doesn't exist. Any files inside
                                   it will be overwritten by similarly named ones that are copied.
        */
        bool copyDirectoryTo(const File &newDirectory) const;

        //==============================================================================
        /** Used in file searching, to specify whether to return files, directories, or both.
        */
        enum TypesOfFileToFind {
            findDirectories = 1,    /**< Use this flag to indicate that you want to find directories. */
            findFiles = 2,    /**< Use this flag to indicate that you want to find files. */
            findFilesAndDirectories = 3,    /**< Use this flag to indicate that you want to find both files and directories. */
            ignoreHiddenFiles = 4     /**< Add this flag to avoid returning any hidden files in the results. */
        };

        enum class FollowSymlinks {
            /** Requests that a file system traversal should not follow any symbolic links. */
            no,

            /** Requests that a file system traversal may follow symbolic links, but should attempt to
                skip any symbolic links to directories that may cause a cycle.
            */
            noCycles,

            /** Requests that a file system traversal follow all symbolic links. Use with care, as this
                may produce inconsistent results, or fail to terminate, if the filesystem contains cycles
                due to symbolic links.
            */
            yes
        };

        /** Searches this directory for files matching a wildcard pattern.

            Assuming that this file is a directory, this method will search it
            for either files or subdirectories whose names match a filename pattern.
            Note that the order in which files are returned is completely undefined!

            @param whatToLookFor            a value from the TypesOfFileToFind enum, specifying whether to
                                            return files, directories, or both. If the ignoreHiddenFiles flag
                                            is also added to this value, hidden files won't be returned
            @param searchRecursively        if true, all subdirectories will be recursed into to do
                                            an exhaustive search
            @param wildCardPattern          the filename pattern to search for, e.g. "*.txt"
            @param followSymlinks           the method that should be used to handle symbolic links
            @returns                        the set of files that were found

            @see getNumberOfChildFiles, RangedDirectoryIterator
        */
        Array<File> findChildFiles(int whatToLookFor,
                                   bool searchRecursively,
                                   const String &wildCardPattern = "*",
                                   FollowSymlinks followSymlinks = FollowSymlinks::yes) const;

        /** Searches inside a directory for files matching a wildcard pattern.
            Note that there's a newer, better version of this method which returns the results
            array, and in almost all cases, you should use that one instead! This one is kept around
            mainly for legacy code to use.
        */
        int findChildFiles(Array<File> &results, int whatToLookFor,
                           bool searchRecursively, const String &wildCardPattern = "*",
                           FollowSymlinks followSymlinks = FollowSymlinks::yes) const;

        /** Searches inside a directory and counts how many files match a wildcard pattern.

            Assuming that this file is a directory, this method will search it
            for either files or subdirectories whose names match a filename pattern,
            and will return the number of matches found.

            This isn't a recursive call, and will only search this directory, not
            its children.

            @param whatToLookFor    a value from the TypesOfFileToFind enum, specifying whether to
                                    count files, directories, or both. If the ignoreHiddenFiles flag
                                    is also added to this value, hidden files won't be counted
            @param wildCardPattern  the filename pattern to search for, e.g. "*.txt"
            @returns                the number of matches found

            @see findChildFiles, RangedDirectoryIterator
        */
        int getNumberOfChildFiles(int whatToLookFor,
                                  const String &wildCardPattern = "*") const;

        /** Returns true if this file is a directory that contains one or more subdirectories.
            @see isDirectory, findChildFiles
        */
        bool containsSubDirectories() const;

        //==============================================================================
        /** Creates a stream to read from this file.

            Note that this is an old method, and actually it's usually best to avoid it and
            instead use an RAII pattern with an FileInputStream directly, e.g.
            @code
            FileInputStream input (fileToOpen);

            if (input.openedOk())
            {
                input.read (etc...
            }
            @endcode

            @returns    a stream that will read from this file (initially positioned at the
                        start of the file), or nullptr if the file can't be opened for some reason
            @see createOutputStream, loadFileAsData
        */
        std::unique_ptr<FileInputStream> createInputStream() const;

        /**
         * 读取文件到bytebuffer，默认以只读模式打开，
         * 内部通过 toMemoryFile 转化为 ByteBuffer
         * 如果需要以读写模式，请使用 toMemoryFile(false) --> ByteBuffer
         * @return
         */
        std::unique_ptr<ByteBuffer> readByteBuffer() const;

        /**
         * 将文件转换为内存映射文件，尽量避免内存浪费
         * @param readOnly  是否以只读的模式打开
         * @return  对象指针
         */
        std::unique_ptr<MemoryFile> toMemoryFile(bool readOnly = true) const;

        /** Creates a stream to write to this file.

            Note that this is an old method, and actually it's usually best to avoid it and
            instead use an RAII pattern with an FileOutputStream directly, e.g.
            @code
            FileOutputStream output (fileToOpen);

            if (output.openedOk())
            {
                output.read etc...
            }
            @endcode

            If the file exists, the stream that is returned will be positioned ready for
            writing at the end of the file. If you want to write to the start of the file,
            replacing the existing content, then you can do the following:
            @code
            FileOutputStream output (fileToOverwrite);

            if (output.openedOk())
            {
                output.setPosition (0);
                output.truncate();
                ...
            }
            @endcode

            @returns    a stream that will write to this file (initially positioned at the
                        end of the file), or nullptr if the file can't be opened for some reason
            @see createInputStream, appendData, appendText
        */
        std::unique_ptr<FileOutputStream> createOutputStream(size_t bufferSize = 0x8000) const;

        //==============================================================================
        /** Loads a file's contents into memory as a block of binary data.

            Of course, trying to load a very large file into memory will blow up, so
            it's better to check first.

            @param result   the data block to which the file's contents should be appended - note
                            that if the memory block might already contain some data, you
                            might want to clear it first
            @returns        true if the file could all be read into memory
        */
        bool loadFileAsData(MemoryBlock &result) const;

        /** Reads a file into memory as a string.

            Attempts to load the entire file as a zero-terminated string.

            This makes use of InputStream::readEntireStreamAsString, which can
            read either UTF-16 or UTF-8 file formats.
        */
        String loadFileAsString() const;

        /** Reads the contents of this file as text and splits it into lines, which are
            appended to the given StringArray.
        */
        void readLines(StringArray &destLines) const;

        //==============================================================================
        /** Appends a block of binary data to the end of the file.

            This will try to write the given buffer to the end of the file.

            @returns false if it can't write to the file for some reason
        */
        bool appendData(const void *dataToAppend,
                        size_t numberOfBytes) const;

        /** Replaces this file's contents with a given block of data.

            This will delete the file and replace it with the given data.

            A nice feature of this method is that it's safe - instead of deleting
            the file first and then re-writing it, it creates a new temporary file,
            writes the data to that, and then moves the new file to replace the existing
            file. This means that if the power gets pulled out or something crashes,
            you're a lot less likely to end up with a corrupted or unfinished file..

            Returns true if the operation succeeds, or false if it fails.

            @see appendText
        */
        bool replaceWithData(const void *dataToWrite,
                             size_t numberOfBytes) const;

        /** Appends a string to the end of the file.

            This will try to append a text string to the file, as either 16-bit unicode
            or 8-bit characters in the default system encoding.

            It can also write the 'ff fe' unicode header bytes before the text to indicate
            the endianness of the file.

            If lineEndings is nullptr, then line endings in the text won't be modified. If you
            pass "\\n" or "\\r\\n" then this function will replace any existing line feeds.

            @see replaceWithText
        */
        bool appendText(const String &textToAppend,
                        bool asUnicode = false,
                        bool writeUnicodeHeaderBytes = false,
                        const char *lineEndings = "\r\n") const;

        /** Replaces this file's contents with a given text string.

            This will delete the file and replace it with the given text.

            A nice feature of this method is that it's safe - instead of deleting
            the file first and then re-writing it, it creates a new temporary file,
            writes the text to that, and then moves the new file to replace the existing
            file. This means that if the power gets pulled out or something crashes,
            you're a lot less likely to end up with an empty file..

            For an explanation of the parameters here, see the appendText() method.

            Returns true if the operation succeeds, or false if it fails.

            @see appendText
        */
        bool replaceWithText(const String &textToWrite,
                             bool asUnicode = false,
                             bool writeUnicodeHeaderBytes = false,
                             const char *lineEndings = "\r\n") const;

        /** Attempts to scan the contents of this file and compare it to another file, returning
            true if this is possible and they match byte-for-byte.
        */
        bool hasIdenticalContentTo(const File &other) const;

        //==============================================================================
        /** Creates a set of files to represent each file root.

            e.g. on Windows this will create files for "c:\", "d:\" etc according
            to which ones are available. On the Mac/Linux, this will probably
            just add a single entry for "/".
        */
        static void findFileSystemRoots(Array<File> &results);

        /** Finds the name of the drive on which this file lives.
            @returns the volume label of the drive, or an empty string if this isn't possible
        */
        String getVolumeLabel() const;

        /** Returns the serial number of the volume on which this file lives.
            @returns the serial number, or zero if there's a problem doing this
        */
        int getVolumeSerialNumber() const;

        /** Returns the number of bytes free on the drive that this file lives on.

            @returns the number of bytes free, or 0 if there's a problem finding this out
            @see getVolumeTotalSize
        */
        int64 getBytesFreeOnVolume() const;

        /** Returns the total size of the drive that contains this file.

            @returns the total number of bytes that the volume can hold
            @see getBytesFreeOnVolume
        */
        int64 getVolumeTotalSize() const;

        /** Returns true if this file is on a CD or DVD drive. */
        bool isOnCDRomDrive() const;

        /** Returns true if this file is on a hard disk.

            This will fail if it's a network drive, but will still be true for
            removable hard-disks.
        */
        bool isOnHardDisk() const;

        /** Returns true if this file is on a removable disk drive.

            This might be a usb-drive, a CD-rom, or maybe a network drive.
        */
        bool isOnRemovableDrive() const;

        //==============================================================================
        /** Launches the file as a process.

            - if the file is executable, this will run it.

            - if it's a document of some kind, it will launch the document with its
            default viewer application.

            - if it's a folder, it will be opened in Explorer, Finder, or equivalent.

            @see revealToUser
        */
        bool startAsProcess(const String &parameters = String()) const;

        /** Opens Finder, Explorer, or whatever the OS uses, to show the user this file's location.
            @see startAsProcess
        */
        void revealToUser() const;

        //==============================================================================
        /** A set of types of location that can be passed to the getSpecialLocation() method.
        */
        enum SpecialLocationType {
            /** The user's home folder. This is the same as using File ("~"). */
            userHomeDirectory,

            /** The user's default documents folder. On Windows, this might be the user's
                "My Documents" folder. On the Mac it'll be their "Documents" folder. Linux
                doesn't tend to have one of these, so it might just return their home folder.
            */
            userDocumentsDirectory,

            /** The folder that contains the user's desktop objects. */
            userDesktopDirectory,

            /** The most likely place where a user might store their music files. */
            userMusicDirectory,

            /** The most likely place where a user might store their movie files. */
            userMoviesDirectory,

            /** The most likely place where a user might store their picture files. */
            userPicturesDirectory,

            /** The folder in which applications store their persistent user-specific settings.
                On Windows, this might be "\Documents and Settings\username\Application Data".
                On the Mac, it might be "~/Library". If you're going to store your settings in here,
                always create your own sub-folder to put them in, to avoid making a mess.
                On GNU/Linux it is "~/.config".
            */
            userApplicationDataDirectory,

            /** An equivalent of the userApplicationDataDirectory folder that is shared by all users
                of the computer, rather than just the current user.

                On the Mac it'll be "/Library", on Windows, it could be something like
                "\Documents and Settings\All Users\Application Data".

                On GNU/Linux it is "/opt".

                Depending on the setup, this folder may be read-only.
            */
            commonApplicationDataDirectory,

            /** A place to put documents which are shared by all users of the machine.
                On Windows this may be somewhere like "C:\Users\Public\Documents", on OSX it
                will be something like "/Users/Shared". Other OSes may have no such concept
                though, so be careful.
            */
            commonDocumentsDirectory,

            /** The folder that should be used for temporary files.
                Always delete them when you're finished, to keep the user's computer tidy!
            */
            tempDirectory,

            /** Returns this application's executable file.

                If running as a plug-in or DLL, this will (where possible) be the DLL rather than the
                host app.

                On the mac this will return the unix binary, not the package folder - see
                currentApplicationFile for that.

                See also invokedExecutableFile, which is similar, but if the exe was launched from a
                file link, invokedExecutableFile will return the name of the link.
            */
            currentExecutableFile,

            currentExecutableDirectory,

            /** Returns this application's location.

                If running as a plug-in or DLL, this will (where possible) be the DLL rather than the
                host app.

                On the mac this will return the package folder (if it's in one), not the unix binary
                that's inside it - compare with currentExecutableFile.
            */
            currentApplicationFile,

            /** Returns the file that was invoked to launch this executable.
                This may differ from currentExecutableFile if the app was started from e.g. a link - this
                will return the name of the link that was used, whereas currentExecutableFile will return
                the actual location of the target executable.
            */
            invokedExecutableFile,

            /** In a plugin, this will return the path of the host executable. */
            hostApplicationPath,

#if STL_WINDOWS || DOXYGEN
            /** On a Windows machine, returns the location of the Windows/System32 folder. */
            windowsSystemDirectory,
#endif

            /** The directory in which applications normally get installed.
                So on windows, this would be something like "C:\Program Files", on the
                Mac "/Applications", or "/usr" on linux.
            */
            globalApplicationsDirectory,

#if STL_WINDOWS || DOXYGEN
            /** On a Windows machine, returns the directory in which 32 bit applications
                normally get installed. On a 64 bit machine this would be something like
                "C:\Program Files (x86)", whereas for 32 bit machines this would match
                globalApplicationsDirectory and be something like "C:\Program Files".

                @see globalApplicationsDirectory
            */
            globalApplicationsDirectoryX86,

            /** On a Windows machine returns the %LOCALAPPDATA% folder. */
            windowsLocalAppData
#endif
        };

        /** Finds the location of a special type of file or directory, such as a home folder or
            documents folder.

            @see SpecialLocationType
        */
        static File STL_CALLTYPE getSpecialLocation(const SpecialLocationType type);

        //==============================================================================
        /** Returns a temporary file in the system's temp directory.
            This will try to return the name of a non-existent temp file.
            To get the temp folder, you can use getSpecialLocation (File::tempDirectory).
        */
        static File createTempFile(StringRef fileNameEnding);

        //==============================================================================
        /** 返回当前工作目录。
            @see setAsCurrentWorkingDirectory
        */
        static File getCurrentWorkingDirectory();

        /**将当前工作目录设置为此文件。要使其工作，文件必须指向一个有效的目录。

            @returns true if the current directory has been changed.
            @see getCurrentWorkingDirectory
        */
        bool setAsCurrentWorkingDirectory() const;

        //==============================================================================
        /** 系统特定的文件分隔符。
            在Windows上，这将是“\”，在Mac/Linux上，它将是“/”
        */
        static stl_wchar getSeparatorChar();

        /** 系统特定的文件分隔符，作为字符串。
            在Windows上，这将是“\”，在Mac/Linux上，它将是“/”
        */
        static StringRef getSeparatorString();

        //==============================================================================
        /** Returns a version of a filename with any illegal characters removed.

            This will return a copy of the given string after removing characters
            that are not allowed in a legal filename, and possibly shortening the
            string if it's too long.

            Because this will remove slashes, don't use it on an absolute pathname - use
            createLegalPathName() for that.

            @see createLegalPathName
        */
        static String createLegalFileName(const String &fileNameToFix);

        /** Returns a version of a path with any illegal characters removed.

            Similar to createLegalFileName(), but this won't remove slashes, so can
            be used on a complete pathname.

            @see createLegalFileName
        */
        static String createLegalPathName(const String &pathNameToFix);


        static String fileName(const String &pathName);

        /** Indicates whether filenames are case-sensitive on the current operating system. */
        static bool areFileNamesCaseSensitive();

        /** Returns true if the string seems to be a fully-specified absolute path. */
        static bool isAbsolutePath(StringRef path);

        /** Creates a file that simply contains this string, without doing the sanity-checking
            that the normal constructors do.

            Best to avoid this unless you really know what you're doing.
        */
        static File createFileWithoutCheckingPath(const String &absolutePath) noexcept;

        /** Adds a separator character to the end of a path if it doesn't already have one. */
        static String addTrailingSeparator(const String &path);

        //==============================================================================
        /** Tries to create a symbolic link and returns a boolean to indicate success */
        bool createSymbolicLink(const File &linkFileToCreate, bool overwriteExisting) const;

        /** Returns true if this file is a link or alias that can be followed using getLinkedTarget(). */
        bool isSymbolicLink() const;

        /** If this file is a link or alias, this returns the file that it points to.
            If the file isn't actually link, it'll just return itself.
        */
        File getLinkedTarget() const;

        /** Create a symbolic link to a native path and return a boolean to indicate success.

            Use this method if you want to create a link to a relative path or a special native
            file path (such as a device file on Windows).
        */
        static bool createSymbolicLink(const File &linkFileToCreate,
                                       const String &nativePathOfTarget,
                                       bool overwriteExisting);

        /** This returns the native path that the symbolic link points to. The returned path
            is a native path of the current OS and can be a relative, absolute or special path. */
        String getNativeLinkedTarget() const;

#if STL_WINDOWS || DOXYGEN
        /** Windows ONLY - Creates a win32 .LNK shortcut file that links to this file. */
        bool createShortcut (const String& description, const File& linkFileToCreate) const;

        /** Windows ONLY - Returns true if this is a win32 .LNK file. */
        bool isShortcut() const;
#else

#endif

        //==============================================================================
#if STL_MAC || STL_IOS || DOXYGEN
        /** OSX ONLY - Finds the OSType of a file from the its resources. */
        OSType getMacOSType() const;

        /** OSX ONLY - Returns true if this file is actually a bundle. */
        bool isBundle() const;
#endif

#if STL_MAC || DOXYGEN
        /** OSX ONLY - Adds this file to the OSX dock */
        void addToDock() const;
#endif

#if STL_MAC || STL_IOS
        /** Returns the path to the container shared by all apps with the provided app group ID.

            You *must* pass one of the app group IDs listed in your app's entitlements file.

            On failure, this function may return a non-existent file, so you should check
            that the path exists and is writable before trying to use it.
        */
        static File getContainerForSecurityApplicationGroupIdentifier (const String& appGroup);
#endif

        //==============================================================================
        /** Comparator for files */
        struct NaturalFileComparator {
            NaturalFileComparator(bool shouldPutFoldersFirst) noexcept: foldersFirst(shouldPutFoldersFirst) {}

            int compareElements(const File &firstFile, const File &secondFile) const {
                if (foldersFirst && (firstFile.isDirectory() != secondFile.isDirectory()))
                    return firstFile.isDirectory() ? -1 : 1;

#if NAMES_ARE_CASE_SENSITIVE
                return firstFile.getFullPathName().compareNatural (secondFile.getFullPathName(), true);
#else
                return firstFile.getFullPathName().compareNatural(secondFile.getFullPathName(), false);
#endif
            }

            bool foldersFirst;
        };

#if STL_ALLOW_STATIC_NULL_VARIABLES && !defined (DOXYGEN)
        /* These static objects are deprecated because it's too easy to accidentally use them indirectly
           during a static constructor, which leads to very obscure order-of-initialisation bugs.
           Use File::getSeparatorChar() and File::getSeparatorString(), and instead of File::nonexistent,
           just use File() or {}.
        */
        [[deprecated]] static const stl_wchar separator;
        [[deprecated]] static const StringRef separatorString;
        [[deprecated]] static const File nonexistent;
#endif

    private:
        //==============================================================================
        String fullPath;

        static String parseAbsolutePath(const String &);

        String getPathUpToLastSlash() const;

        Result createDirectoryInternal(const String &) const;

        bool copyInternal(const File &) const;

        bool moveInternal(const File &) const;

        bool replaceInternal(const File &) const;

        bool setFileTimesInternal(int64 m, int64 a, int64 c) const;

        void getFileTimesInternal(int64 &m, int64 &a, int64 &c) const;

        bool setFileReadOnlyInternal(bool) const;

        bool setFileExecutableInternal(bool) const;
    };

} // namespace stl
