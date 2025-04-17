#pragma once

namespace stl
{

#if ! (DOXYGEN || STL_EXCEPTIONS_DISABLED)
namespace HeapBlockHelper
{
    template <bool shouldThrow>
    struct ThrowOnFail          { static void checkPointer (void*) {} };

    template <>
    struct ThrowOnFail<true>    { static void checkPointer (void* data) { if (data == nullptr) throw std::bad_alloc(); } };
}
#endif

//==============================================================================
/**
    非常简单的容器类，用于保存指向堆上某些数据的指针。
    当您需要为某些东西分配一些堆存储时，请始终尝试使用
    这个类，而不是直接使用malloc/free来分配内存。
    HeapBlock＜char＞对象可以用几乎完全相同的方式处理
    作为char*，但只要您在堆栈上或作为类成员分配它，
    它几乎不可能泄露内存。
    它还使您的代码比做同样的事情更加简洁和可读
    使用直接分配，

    E.g. instead of this:
    @code
        int* temp = (int*) malloc (1024 * sizeof (int));
        memcpy (temp, xyz, 1024 * sizeof (int));
        free (temp);
        temp = (int*) calloc (2048 * sizeof (int));
        temp[0] = 1234;
        memcpy (foobar, temp, 2048 * sizeof (int));
        free (temp);
    @endcode

    ..you could just write this:
    @code
        HeapBlock<int> temp (1024);
        memcpy (temp, xyz, 1024 * sizeof (int));
        temp.calloc (2048);
        temp[0] = 1234;
        memcpy (foobar, temp, 2048 * sizeof (int));
    @endcode

    该类非常轻量级，只包含一个指向
    数据，并公开执行相同工作的malloc/realloc/free方法
    作为不太面向对象的对应产品。尽管增加了安全性，但你可能
    不会因为使用它来代替普通指针而牺牲任何性能。
    如果您想要类，throwOnFailure模板参数可以设置为true
    在分配失败时抛出std:：bad_alloc异常。如果这是错误的，
    那么失败的分配将只留下一个空指针的堆块（假设
    系统的malloc（）函数不会抛出）。

    @see Array, OwnedArray, MemoryBlock

    @tags{Core}
*/
template <class ElementType, bool throwOnFailure = false>
class HeapBlock
{
private:
    template <class OtherElementType>
    using AllowConversion = std::enable_if_t<std::is_base_of_v<std::remove_pointer_t<ElementType>,
                                                               std::remove_pointer_t<OtherElementType>>>;

public:
    //==============================================================================
    /** Creates a HeapBlock which is initially just a null pointer.

        After creation, you can resize the array using the malloc(), calloc(),
        or realloc() methods.
    */
    HeapBlock() = default;

    /** Creates a HeapBlock containing a number of elements.

        The contents of the block are undefined, as it will have been created by a
        malloc call.

        If you want an array of zero values, you can use the calloc() method or the
        other constructor that takes an InitialisationState parameter.
    */
    template <typename SizeType, std::enable_if_t<std::is_convertible_v<SizeType, int>, int> = 0>
    explicit HeapBlock (SizeType numElements)
        : data (mallocWrapper (static_cast<size_t> (numElements) * sizeof (ElementType)))
    {
    }

    /** Creates a HeapBlock containing a number of elements.

        The initialiseToZero parameter determines whether the new memory should be cleared,
        or left uninitialised.
    */
    template <typename SizeType, std::enable_if_t<std::is_convertible_v<SizeType, int>, int> = 0>
    HeapBlock (SizeType numElements, bool initialiseToZero)
        : data (initialiseToZero ? callocWrapper (static_cast<size_t> (numElements), sizeof (ElementType))
                                 : mallocWrapper (static_cast<size_t> (numElements) * sizeof (ElementType)))
    {
    }

    /** Destructor.
        This will free the data, if any has been allocated.
    */
    ~HeapBlock()
    {
        std::free (data);
    }

    /** Move constructor */
    HeapBlock (HeapBlock&& other) noexcept
        : data (other.data)
    {
        other.data = nullptr;
    }

    /** Move assignment operator */
    HeapBlock& operator= (HeapBlock&& other) noexcept
    {
        std::swap (data, other.data);
        return *this;
    }

    /** Converting move constructor.
        Only enabled if this is a HeapBlock<Base*> and the other object is a HeapBlock<Derived*>,
        where std::is_base_of_v<Base, Derived> == true.
    */
    template <class OtherElementType, bool otherThrowOnFailure, typename = AllowConversion<OtherElementType>>
    HeapBlock (HeapBlock<OtherElementType, otherThrowOnFailure>&& other) noexcept
        : data (reinterpret_cast<ElementType*> (other.data))
    {
        other.data = nullptr;
    }

    /** Converting move assignment operator.
        Only enabled if this is a HeapBlock<Base*> and the other object is a HeapBlock<Derived*>,
        where std::is_base_of_v<Base, Derived> == true.
    */
    template <class OtherElementType, bool otherThrowOnFailure, typename = AllowConversion<OtherElementType>>
    HeapBlock& operator= (HeapBlock<OtherElementType, otherThrowOnFailure>&& other) noexcept
    {
        free();
        data = reinterpret_cast<ElementType*> (other.data);
        other.data = nullptr;
        return *this;
    }

    //==============================================================================
    /** Returns a raw pointer to the allocated data.
        This may be a null pointer if the data hasn't yet been allocated, or if it has been
        freed by calling the free() method.
    */
    inline operator ElementType*() const noexcept                            { return data; }

    /** Returns a raw pointer to the allocated data.
        This may be a null pointer if the data hasn't yet been allocated, or if it has been
        freed by calling the free() method.
    */
    inline ElementType* get() const noexcept                                 { return data; }

    /** Returns a raw pointer to the allocated data.
        This may be a null pointer if the data hasn't yet been allocated, or if it has been
        freed by calling the free() method.
    */
    inline ElementType* getData() const noexcept                             { return data; }

    /** Returns a void pointer to the allocated data.
        This may be a null pointer if the data hasn't yet been allocated, or if it has been
        freed by calling the free() method.
    */
    inline operator void*() const noexcept                                   { return static_cast<void*> (data); }

    /** Returns a void pointer to the allocated data.
        This may be a null pointer if the data hasn't yet been allocated, or if it has been
        freed by calling the free() method.
    */
    inline operator const void*() const noexcept                             { return static_cast<const void*> (data); }

    /** Lets you use indirect calls to the first element in the array.
        Obviously this will cause problems if the array hasn't been initialised, because it'll
        be referencing a null pointer.
    */
    inline ElementType* operator->() const  noexcept                         { return data; }

    /** Returns a reference to one of the data elements.
        Obviously there's no bounds-checking here, as this object is just a dumb pointer and
        has no idea of the size it currently has allocated.
    */
    template <typename IndexType>
    ElementType& operator[] (IndexType index) const noexcept                 { return data [index]; }

    /** Returns a pointer to a data element at an offset from the start of the array.
        This is the same as doing pointer arithmetic on the raw pointer itself.
    */
    template <typename IndexType>
    ElementType* operator+ (IndexType index) const noexcept                  { return data + index; }

    //==============================================================================
    /** Compares the pointer with another pointer.
        This can be handy for checking whether this is a null pointer.
    */
    inline bool operator== (const ElementType* otherPointer) const noexcept  { return otherPointer == data; }

    /** Compares the pointer with another pointer.
        This can be handy for checking whether this is a null pointer.
    */
    inline bool operator!= (const ElementType* otherPointer) const noexcept  { return otherPointer != data; }

    //==============================================================================
    /** Allocates a specified amount of memory.

        This uses the normal malloc to allocate an amount of memory for this object.
        Any previously allocated memory will be freed by this method.

        The number of bytes allocated will be (newNumElements * elementSize). Normally
        you wouldn't need to specify the second parameter, but it can be handy if you need
        to allocate a size in bytes rather than in terms of the number of elements.

        The data that is allocated will be freed when this object is deleted, or when you
        call free() or any of the allocation methods.
    */
    template <typename SizeType>
    void malloc (SizeType newNumElements, size_t elementSize = sizeof (ElementType))
    {
        std::free (data);
        data = mallocWrapper (static_cast<size_t> (newNumElements) * elementSize);
    }

    /** Allocates a specified amount of memory and clears it.
        This does the same job as the malloc() method, but clears the memory that it allocates.
    */
    template <typename SizeType>
    void calloc (SizeType newNumElements, const size_t elementSize = sizeof (ElementType))
    {
        std::free (data);
        data = callocWrapper (static_cast<size_t> (newNumElements), elementSize);
    }

    /** Allocates a specified amount of memory and optionally clears it.
        This does the same job as either malloc() or calloc(), depending on the
        initialiseToZero parameter.
    */
    template <typename SizeType>
    void allocate (SizeType newNumElements, bool initialiseToZero)
    {
        std::free (data);
        data = initialiseToZero ? callocWrapper (static_cast<size_t> (newNumElements), sizeof (ElementType))
                                : mallocWrapper (static_cast<size_t> (newNumElements) * sizeof (ElementType));
    }

    /** Re-allocates a specified amount of memory.

        The semantics of this method are the same as malloc() and calloc(), but it
        uses realloc() to keep as much of the existing data as possible.
    */
    template <typename SizeType>
    void realloc (SizeType newNumElements, size_t elementSize = sizeof (ElementType))
    {
        data = reallocWrapper (data, static_cast<size_t> (newNumElements) * elementSize);
    }

    /** Frees any currently-allocated data.
        This will free the data and reset this object to be a null pointer.
    */
    void free() noexcept
    {
        std::free (data);
        data = nullptr;
    }

    /** Swaps this object's data with the data of another HeapBlock.
        The two objects simply exchange their data pointers.
    */
    template <bool otherBlockThrows>
    void swapWith (HeapBlock<ElementType, otherBlockThrows>& other) noexcept
    {
        std::swap (data, other.data);
    }

    /** This fills the block with zeros, up to the number of elements specified.
        Since the block has no way of knowing its own size, you must make sure that the number of
        elements you specify doesn't exceed the allocated size.
    */
    template <typename SizeType>
    void clear (SizeType numElements) noexcept
    {
        zeromem (data, sizeof (ElementType) * static_cast<size_t> (numElements));
    }

    /** This typedef can be used to get the type of the heapblock's elements. */
    using Type = ElementType;

private:
    //==============================================================================
    // Calls to malloc, calloc and realloc with zero size have implementation-defined
    // behaviour where either nullptr or a non-null pointer is returned.
    template <typename Functor>
    static ElementType* wrapper (size_t size, Functor&& f)
    {
        if (size == 0)
            return nullptr;

        auto* memory = static_cast<ElementType*> (f());

       #if STL_EXCEPTIONS_DISABLED
        jassert (memory != nullptr); // without exceptions, you'll need to find a better way to handle this failure case.
       #else
        HeapBlockHelper::ThrowOnFail<throwOnFailure>::checkPointer (memory);
       #endif

        return memory;
    }

    static ElementType* mallocWrapper (size_t size)
    {
        return wrapper (size, [size] { return std::malloc (size); });
    }

    static ElementType* callocWrapper (size_t num, size_t size)
    {
        return wrapper (num * size, [num, size] { return std::calloc (num, size); });
    }

    static ElementType* reallocWrapper (void* ptr, size_t newSize)
    {
        return wrapper (newSize, [ptr, newSize] { return std::realloc (ptr, newSize); });
    }

    template <class OtherElementType, bool otherThrowOnFailure>
    friend class HeapBlock;

    //==============================================================================
    ElementType* data = nullptr;

   #if ! (defined (STL_DLL) || defined (STL_DLL_BUILD))
    STL_DECLARE_NON_COPYABLE (HeapBlock)
    STL_PREVENT_HEAP_ALLOCATION // Creating a 'new HeapBlock' would be missing the point!
   #endif
};

} // namespace stl
