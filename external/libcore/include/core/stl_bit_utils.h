//
// Created by xb on 2023/12/18.
//

#ifndef MANXI_CORE_BIT_UTILS_H
#define MANXI_CORE_BIT_UTILS_H
#include <limits>
#include <type_traits>
#include "stl_macro.h"
#include "logging/check.h"

namespace stl{

// Like sizeof, but count how many bits a type takes. Pass type explicitly.
    template <typename T>
    constexpr size_t BitSizeOf() {
        static_assert(std::is_integral_v<T>, "T must be integral");
        using unsigned_type = std::make_unsigned_t<T>;
        static_assert(sizeof(T) == sizeof(unsigned_type), "Unexpected type size mismatch!");
        static_assert(std::numeric_limits<unsigned_type>::radix == 2, "Unexpected radix!");
        return std::numeric_limits<unsigned_type>::digits;
    }

// Like sizeof, but count how many bits a type takes. Infers type from parameter.
    template <typename T>
    constexpr size_t BitSizeOf(T /*x*/) {
        return BitSizeOf<T>();
    }

    template<typename T>
    constexpr int CLZ(T x) {
        static_assert(std::is_integral_v<T>, "T must be integral");
        static_assert(std::is_unsigned_v<T>, "T must be unsigned");
        static_assert(std::numeric_limits<T>::radix == 2, "Unexpected radix!");
        static_assert(sizeof(T) == sizeof(uint64_t) || sizeof(T) <= sizeof(uint32_t),
                      "Unsupported sizeof(T)");
        DCHECK_NE(x, 0u);
        constexpr bool is_64_bit = (sizeof(T) == sizeof(uint64_t));
        constexpr size_t adjustment =
                is_64_bit ? 0u : std::numeric_limits<uint32_t>::digits - std::numeric_limits<T>::digits;
        return is_64_bit ? __builtin_clzll(x) : __builtin_clz(x) - adjustment;
    }

// Similar to CLZ except that on zero input it returns bitwidth and supports signed integers.
    template<typename T>
    constexpr int JAVASTYLE_CLZ(T x) {
        static_assert(std::is_integral_v<T>, "T must be integral");
        using unsigned_type = std::make_unsigned_t<T>;
        return (x == 0) ? BitSizeOf<T>() : CLZ(static_cast<unsigned_type>(x));
    }

    template<typename T>
    constexpr int CTZ(T x) {
        static_assert(std::is_integral_v<T>, "T must be integral");
        // It is not unreasonable to ask for trailing zeros in a negative number. As such, do not check
        // that T is an unsigned type.
        static_assert(sizeof(T) == sizeof(uint64_t) || sizeof(T) <= sizeof(uint32_t),
                      "Unsupported sizeof(T)");
        DCHECK_NE(x, static_cast<T>(0));
        return (sizeof(T) == sizeof(uint64_t)) ? __builtin_ctzll(x) : __builtin_ctz(x);
    }

// Similar to CTZ except that on zero input it returns bitwidth and supports signed integers.
    template<typename T>
    constexpr int JAVASTYLE_CTZ(T x) {
        static_assert(std::is_integral_v<T>, "T must be integral");
        using unsigned_type = std::make_unsigned_t<T>;
        return (x == 0) ? BitSizeOf<T>() : CTZ(static_cast<unsigned_type>(x));
    }

// Return the number of 1-bits in `x`.
    template<typename T>
    constexpr int POPCOUNT(T x) {
        return (sizeof(T) == sizeof(uint32_t)) ? __builtin_popcount(x) : __builtin_popcountll(x);
    }

// Swap bytes.
    template<typename T>
    constexpr T BSWAP(T x) {
        if (sizeof(T) == sizeof(uint16_t)) {
            return __builtin_bswap16(x);
        } else if (sizeof(T) == sizeof(uint32_t)) {
            return __builtin_bswap32(x);
        } else {
            return __builtin_bswap64(x);
        }
    }

// Find the bit position of the most significant bit (0-based), or -1 if there were no bits set.
    template <typename T>
    constexpr ssize_t MostSignificantBit(T value) {
        static_assert(std::is_integral_v<T>, "T must be integral");
        static_assert(std::is_unsigned_v<T>, "T must be unsigned");
        static_assert(std::numeric_limits<T>::radix == 2, "Unexpected radix!");
        return (value == 0) ? -1 : std::numeric_limits<T>::digits - 1 - CLZ(value);
    }

// Find the bit position of the least significant bit (0-based), or -1 if there were no bits set.
    template <typename T>
    constexpr ssize_t LeastSignificantBit(T value) {
        static_assert(std::is_integral_v<T>, "T must be integral");
        static_assert(std::is_unsigned_v<T>, "T must be unsigned");
        return (value == 0) ? -1 : CTZ(value);
    }

// How many bits (minimally) does it take to store the constant 'value'? i.e. 1 for 1, 3 for 5, etc.
    template <typename T>
    constexpr size_t MinimumBitsToStore(T value) {
        return static_cast<size_t>(MostSignificantBit(value) + 1);
    }

    template <typename T>
    constexpr T RoundUpToPowerOfTwo(T x) {
        static_assert(std::is_integral_v<T>, "T must be integral");
        static_assert(std::is_unsigned_v<T>, "T must be unsigned");
        // NOTE: Undefined if x > (1 << (std::numeric_limits<T>::digits - 1)).
        return (x < 2u) ? x : static_cast<T>(1u) << (std::numeric_limits<T>::digits - CLZ(x - 1u));
    }

// Return highest possible N - a power of two - such that val >= N.
    template <typename T>
    constexpr T TruncToPowerOfTwo(T val) {
        static_assert(std::is_integral_v<T>, "T must be integral");
        static_assert(std::is_unsigned_v<T>, "T must be unsigned");
        return (val != 0) ? static_cast<T>(1u) << (BitSizeOf<T>() - CLZ(val) - 1u) : 0;
    }

    template<typename T>
    constexpr bool IsPowerOfTwo(T x) {
        static_assert(std::is_integral_v<T>, "T must be integral");
        // TODO: assert unsigned. There is currently many uses with signed values.
        return (x & (x - 1)) == 0;
    }

    template<typename T>
    constexpr int WhichPowerOf2(T x) {
        static_assert(std::is_integral_v<T>, "T must be integral");
        // TODO: assert unsigned. There is currently many uses with signed values.
        DCHECK((x != 0) && IsPowerOfTwo(x));
        return CTZ(x);
    }
// Use to suppress type deduction for a function argument.
// See std::identity<> for more background:
// http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2005/n1856.html#20.2.2 - move/forward helpers
//
// e.g. "template <typename X> void bar(identity<X>::type foo);
//     bar(5); // compilation error
//     bar<int>(5); // ok
// or "template <typename T> void foo(T* x, typename Identity<T*>::type y);
//     Base b;
//     Derived d;
//     foo(&b, &d);  // Use implicit Derived* -> Base* conversion.
// If T was deduced from both &b and &d, there would be a mismatch, i.e. deduction failure.
    template <typename T>
    struct Identity {
        using type = T;
    };
// For rounding integers.
// Note: Omit the `n` from T type deduction, deduce only from the `x` argument.
    template<typename T>
    constexpr T RoundDown(T x, typename Identity<T>::type n) ;

    template<typename T>
    constexpr T RoundDown(T x, typename Identity<T>::type n) {
        DCHECK(IsPowerOfTwo(n));
        return (x & -n);
    }

    template<typename T>
    constexpr T RoundUp(T x, std::remove_reference_t<T> n) ;

    template<typename T>
    constexpr T RoundUp(T x, std::remove_reference_t<T> n) {
        return RoundDown(x + n - 1, n);
    }

    template<bool kRoundUp, typename T>
    constexpr T CondRoundUp(T x, std::remove_reference_t<T> n) {
        if (kRoundUp) {
            return RoundUp(x, n);
        } else {
            return x;
        }
    }

// For aligning pointers.
    template<typename T>
    inline T* AlignDown(T* x, uintptr_t n) ;

    template<typename T>
    inline T* AlignDown(T* x, uintptr_t n) {
        return reinterpret_cast<T*>(RoundDown(reinterpret_cast<uintptr_t>(x), n));
    }

    template<typename T>
    inline T* AlignUp(T* x, uintptr_t n) ;

    template<typename T>
    inline T* AlignUp(T* x, uintptr_t n) {
        return reinterpret_cast<T*>(RoundUp(reinterpret_cast<uintptr_t>(x), n));
    }

    template<int n, typename T>
    constexpr bool IsAligned(T x) {
        static_assert((n & (n - 1)) == 0, "n is not a power of two");
        return (x & (n - 1)) == 0;
    }

    template<int n, typename T>
    inline bool IsAligned(T* x) {
        return IsAligned<n>(reinterpret_cast<const uintptr_t>(x));
    }

    template<typename T>
    inline bool IsAlignedParam(T x, int n) {
        return (x & (n - 1)) == 0;
    }

    template<typename T>
    inline bool IsAlignedParam(T* x, int n) {
        return IsAlignedParam(reinterpret_cast<const uintptr_t>(x), n);
    }

#define CHECK_ALIGNED(value, alignment) \
  CHECK(::stl::IsAligned<alignment>(value)) << reinterpret_cast<const void*>(value)

#define DCHECK_ALIGNED(value, alignment) \
  DCHECK(::stl::IsAligned<alignment>(value)) << reinterpret_cast<const void*>(value)

#define CHECK_ALIGNED_PARAM(value, alignment) \
  CHECK(::stl::IsAlignedParam(value, alignment)) << reinterpret_cast<const void*>(value)

#define DCHECK_ALIGNED_PARAM(value, alignment) \
  DCHECK(::stl::IsAlignedParam(value, alignment)) << reinterpret_cast<const void*>(value)

    inline uint16_t Low16Bits(uint32_t value) {
        return static_cast<uint16_t>(value);
    }

    inline uint16_t High16Bits(uint32_t value) {
        return static_cast<uint16_t>(value >> 16);
    }

    inline uint32_t Low32Bits(uint64_t value) {
        return static_cast<uint32_t>(value);
    }

    inline uint32_t High32Bits(uint64_t value) {
        return static_cast<uint32_t>(value >> 32);
    }

// Check whether an N-bit two's-complement representation can hold value.
    template <typename T>
    inline bool IsInt(size_t N, T value) {
        if (N == BitSizeOf<T>()) {
            return true;
        } else {
            CHECK_LT(0u, N);
            CHECK_LT(N, BitSizeOf<T>());
            T limit = static_cast<T>(1) << (N - 1u);
            return (-limit <= value) && (value < limit);
        }
    }

    template <typename T>
    constexpr T GetIntLimit(size_t bits) {
        DCHECK_NE(bits, 0u);
        DCHECK_LT(bits, BitSizeOf<T>());
        return static_cast<T>(1) << (bits - 1);
    }

    template <size_t kBits, typename T>
    constexpr bool IsInt(T value) {
        static_assert(kBits > 0, "kBits cannot be zero.");
        static_assert(kBits <= BitSizeOf<T>(), "kBits must be <= max.");
        static_assert(std::is_signed_v<T>, "Needs a signed type.");
        // Corner case for "use all bits." Can't use the limits, as they would overflow, but it is
        // trivially true.
        return (kBits == BitSizeOf<T>()) ?
               true :
               (-GetIntLimit<T>(kBits) <= value) && (value < GetIntLimit<T>(kBits));
    }

    template <size_t kBits, typename T>
    constexpr bool IsUint(T value) {
        static_assert(kBits > 0, "kBits cannot be zero.");
        static_assert(kBits <= BitSizeOf<T>(), "kBits must be <= max.");
        static_assert(std::is_integral_v<T>, "Needs an integral type.");
        // Corner case for "use all bits." Can't use the limits, as they would overflow, but it is
        // trivially true.
        // NOTE: To avoid triggering assertion in GetIntLimit(kBits+1) if kBits+1==BitSizeOf<T>(),
        // use GetIntLimit(kBits)*2u. The unsigned arithmetic works well for us if it overflows.
        using unsigned_type = std::make_unsigned_t<T>;
        return (0 <= value) &&
               (kBits == BitSizeOf<T>() ||
                (static_cast<unsigned_type>(value) <= GetIntLimit<unsigned_type>(kBits) * 2u - 1u));
    }

    template <size_t kBits, typename T>
    constexpr bool IsAbsoluteUint(T value) {
        static_assert(kBits <= BitSizeOf<T>(), "kBits must be <= max.");
        static_assert(std::is_integral_v<T>, "Needs an integral type.");
        using unsigned_type = std::make_unsigned_t<T>;
        return (kBits == BitSizeOf<T>())
               ? true
               : IsUint<kBits>(value < 0
                               ? static_cast<unsigned_type>(-1 - value) + 1u  // Avoid overflow.
                               : static_cast<unsigned_type>(value));
    }

// Generate maximum/minimum values for signed/unsigned n-bit integers
    template <typename T>
    constexpr T MaxInt(size_t bits) {
        DCHECK(std::is_unsigned_v<T> || bits > 0u) << "bits cannot be zero for signed.";
        DCHECK_LE(bits, BitSizeOf<T>());
        using unsigned_type = std::make_unsigned_t<T>;
        return bits == BitSizeOf<T>()
               ? std::numeric_limits<T>::max()
               : std::is_signed_v<T>
                 ? ((bits == 1u) ? 0 : static_cast<T>(MaxInt<unsigned_type>(bits - 1)))
                 : static_cast<T>(UINT64_C(1) << bits) - static_cast<T>(1);
    }

    template <typename T>
    constexpr T MinInt(size_t bits) {
        DCHECK(std::is_unsigned_v<T> || bits > 0) << "bits cannot be zero for signed.";
        DCHECK_LE(bits, BitSizeOf<T>());
        return bits == BitSizeOf<T>()
               ? std::numeric_limits<T>::min()
               : std::is_signed_v<T>
                 ? ((bits == 1u) ? -1 : static_cast<T>(-1) - MaxInt<T>(bits))
                 : static_cast<T>(0);
    }

// Returns value with bit set in lowest one-bit position or 0 if 0.  (java.lang.X.lowestOneBit).
    template <typename kind>
    inline static kind LowestOneBitValue(kind opnd) {
        // Hacker's Delight, Section 2-1
        return opnd & -opnd;
    }

// Returns value with bit set in hightest one-bit position or 0 if 0.  (java.lang.X.highestOneBit).
    template <typename T>
    inline static T HighestOneBitValue(T opnd) {
        using unsigned_type = std::make_unsigned_t<T>;
        T res;
        if (opnd == 0) {
            res = 0;
        } else {
            int bit_position = BitSizeOf<T>() - (CLZ(static_cast<unsigned_type>(opnd)) + 1);
            res = static_cast<T>(UINT64_C(1) << bit_position);
        }
        return res;
    }

// Rotate bits.
    template <typename T, bool left>
    inline static T Rot(T opnd, int distance) {
        int mask = BitSizeOf<T>() - 1;
        int unsigned_right_shift = left ? (-distance & mask) : (distance & mask);
        int signed_left_shift = left ? (distance & mask) : (-distance & mask);
        using unsigned_type = std::make_unsigned_t<T>;
        return (static_cast<unsigned_type>(opnd) >> unsigned_right_shift) | (opnd << signed_left_shift);
    }

// TUNING: use rbit for arm/arm64
    inline static uint32_t ReverseBits32(uint32_t opnd) {
        // Hacker's Delight 7-1
        opnd = ((opnd >>  1) & 0x55555555) | ((opnd & 0x55555555) <<  1);
        opnd = ((opnd >>  2) & 0x33333333) | ((opnd & 0x33333333) <<  2);
        opnd = ((opnd >>  4) & 0x0F0F0F0F) | ((opnd & 0x0F0F0F0F) <<  4);
        opnd = ((opnd >>  8) & 0x00FF00FF) | ((opnd & 0x00FF00FF) <<  8);
        opnd = ((opnd >> 16)) | ((opnd) << 16);
        return opnd;
    }

// TUNING: use rbit for arm/arm64
    inline static uint64_t ReverseBits64(uint64_t opnd) {
        // Hacker's Delight 7-1
        opnd = (opnd & 0x5555555555555555L) << 1 | ((opnd >> 1) & 0x5555555555555555L);
        opnd = (opnd & 0x3333333333333333L) << 2 | ((opnd >> 2) & 0x3333333333333333L);
        opnd = (opnd & 0x0f0f0f0f0f0f0f0fL) << 4 | ((opnd >> 4) & 0x0f0f0f0f0f0f0f0fL);
        opnd = (opnd & 0x00ff00ff00ff00ffL) << 8 | ((opnd >> 8) & 0x00ff00ff00ff00ffL);
        opnd = (opnd << 48) | ((opnd & 0xffff0000L) << 16) | ((opnd >> 16) & 0xffff0000L) | (opnd >> 48);
        return opnd;
    }

// Create a mask for the least significant "bits"
// The returned value is always unsigned to prevent undefined behavior for bitwise ops.
//
// Given 'bits',
// Returns:
//                   <--- bits --->
// +-----------------+------------+
// | 0 ............0 |   1.....1  |
// +-----------------+------------+
// msb                           lsb
    template <typename T = size_t>
    inline static constexpr std::make_unsigned_t<T> MaskLeastSignificant(size_t bits) {
        DCHECK_GE(BitSizeOf<T>(), bits) << "Bits out of range for type T";
        using unsigned_T = std::make_unsigned_t<T>;
        if (bits >= BitSizeOf<T>()) {
            return std::numeric_limits<unsigned_T>::max();
        } else {
            auto kOne = static_cast<unsigned_T>(1);  // Do not truncate for T>size_t.
            return static_cast<unsigned_T>((kOne << bits) - kOne);
        }
    }

// Clears the bitfield starting at the least significant bit "lsb" with a bitwidth of 'width'.
// (Equivalent of ARM BFC instruction).
//
// Given:
//           <-- width  -->
// +--------+------------+--------+
// | ABC... |  bitfield  | XYZ... +
// +--------+------------+--------+
//                       lsb      0
// Returns:
//           <-- width  -->
// +--------+------------+--------+
// | ABC... | 0........0 | XYZ... +
// +--------+------------+--------+
//                       lsb      0
    template <typename T>
    inline static constexpr T BitFieldClear(T value, size_t lsb, size_t width) {
        DCHECK_GE(BitSizeOf(value), lsb + width) << "Bit field out of range for value";
        const auto val = static_cast<std::make_unsigned_t<T>>(value);
        const auto mask = MaskLeastSignificant<T>(width);

        return static_cast<T>(val & ~(mask << lsb));
    }

// Inserts the contents of 'data' into bitfield of 'value'  starting
// at the least significant bit "lsb" with a bitwidth of 'width'.
// Note: data must be within range of [MinInt(width), MaxInt(width)].
// (Equivalent of ARM BFI instruction).
//
// Given (data):
//           <-- width  -->
// +--------+------------+--------+
// | ABC... |  bitfield  | XYZ... +
// +--------+------------+--------+
//                       lsb      0
// Returns:
//           <-- width  -->
// +--------+------------+--------+
// | ABC... | 0...data   | XYZ... +
// +--------+------------+--------+
//                       lsb      0

    template <typename T, typename T2>
    inline static constexpr T BitFieldInsert(T value, T2 data, size_t lsb, size_t width) {
        DCHECK_GE(BitSizeOf(value), lsb + width) << "Bit field out of range for value";
        if (width != 0u) {
            DCHECK_GE(MaxInt<T2>(width), data) << "Data out of range [too large] for bitwidth";
            DCHECK_LE(MinInt<T2>(width), data) << "Data out of range [too small] for bitwidth";
        } else {
            DCHECK_EQ(static_cast<T2>(0), data) << "Data out of range [nonzero] for bitwidth 0";
        }
        const auto data_mask = MaskLeastSignificant<T2>(width);
        const auto value_cleared = BitFieldClear(value, lsb, width);

        return static_cast<T>(value_cleared | ((data & data_mask) << lsb));
    }

// Extracts the bitfield starting at the least significant bit "lsb" with a bitwidth of 'width'.
// Signed types are sign-extended during extraction. (Equivalent of ARM UBFX/SBFX instruction).
//
// Given:
//           <-- width   -->
// +--------+-------------+-------+
// |        |   bitfield  |       +
// +--------+-------------+-------+
//                       lsb      0
// (Unsigned) Returns:
//                  <-- width   -->
// +----------------+-------------+
// | 0...        0  |   bitfield  |
// +----------------+-------------+
//                                0
// (Signed) Returns:
//                  <-- width   -->
// +----------------+-------------+
// | S...        S  |   bitfield  |
// +----------------+-------------+
//                                0
// where S is the highest bit in 'bitfield'.
    template <typename T>
    inline static constexpr T BitFieldExtract(T value, size_t lsb, size_t width) {
        DCHECK_GE(BitSizeOf(value), lsb + width) << "Bit field out of range for value";
        const auto val = static_cast<std::make_unsigned_t<T>>(value);

        const T bitfield_unsigned =
                static_cast<T>((val >> lsb) & MaskLeastSignificant<T>(width));
        if (std::is_signed_v<T>) {
            // Perform sign extension
            if (width == 0) {  // Avoid underflow.
                return static_cast<T>(0);
            } else if (bitfield_unsigned & (1 << (width - 1))) {  // Detect if sign bit was set.
                // MSB        <width> LSB
                // 0b11111...100...000000
                const auto ones_negmask = ~MaskLeastSignificant<T>(width);
                return static_cast<T>(bitfield_unsigned | ones_negmask);
            }
        }
        // Skip sign extension.
        return bitfield_unsigned;
    }

    inline static constexpr size_t BitsToBytesRoundUp(size_t num_bits) {
        return RoundUp(num_bits, kBitsPerByte) / kBitsPerByte;
    }


    // Helper class that acts as a container for range-based loops, given an iteration
// range [first, last) defined by two iterators.
    template <typename Iter>
    class IterationRange {
    public:
        using iterator        = Iter;
        using difference_type = typename std::iterator_traits<Iter>::difference_type;
        using value_type      = typename std::iterator_traits<Iter>::value_type;
        using pointer         = typename std::iterator_traits<Iter>::pointer;
        using reference       = typename std::iterator_traits<Iter>::reference;

        IterationRange(iterator first, iterator last) : first_(first), last_(last) { }

        iterator begin() const { return first_; }
        iterator end() const { return last_; }
        iterator cbegin() const { return first_; }
        iterator cend() const { return last_; }

    protected:
        iterator first_;
        iterator last_;
    };

    template <typename Iter>
    inline IterationRange<Iter> MakeIterationRange(const Iter& begin_it, const Iter& end_it) {
        return IterationRange<Iter>(begin_it, end_it);
    }

    template <typename List>
    inline auto MakeIterationRange(List& list) -> IterationRange<decltype(list.begin())> {
        static_assert(std::is_same_v<decltype(list.begin()), decltype(list.end())>,
                      "Different iterator types");
        return MakeIterationRange(list.begin(), list.end());
    }

    template <typename Iter>
    inline IterationRange<Iter> MakeEmptyIterationRange(const Iter& it) {
        return IterationRange<Iter>(it, it);
    }

    template <typename Container>
    inline auto ReverseRange(Container&& c) {
        using riter = typename std::reverse_iterator<decltype(c.begin())>;
        return MakeIterationRange(riter(c.end()), riter(c.begin()));
    }

    template <typename T, size_t size>
    inline auto ReverseRange(T (&array)[size]) {
        return ReverseRange(MakeIterationRange<T*>(array, array+size));
    }



    // Using the Curiously Recurring Template Pattern to implement everything shared
// by LowToHighBitIterator and HighToLowBitIterator, i.e. everything but operator*().
    template <typename T, typename Iter>
    class BitIteratorBase {
        static_assert(std::is_integral_v<T>, "T must be integral");
        static_assert(std::is_unsigned_v<T>, "T must be unsigned");

        static_assert(sizeof(T) == sizeof(uint32_t) || sizeof(T) == sizeof(uint64_t), "Unsupported size");

    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = uint32_t;
        using difference_type = ptrdiff_t;
        using pointer = void;
        using reference = void;

        BitIteratorBase() : bits_(0u) { }
        explicit BitIteratorBase(T bits) : bits_(bits) { }

        Iter& operator++() {
            DCHECK_NE(bits_, 0u);
            uint32_t bit = *static_cast<Iter&>(*this);
            bits_ &= ~(static_cast<T>(1u) << bit);
            return static_cast<Iter&>(*this);
        }

        Iter& operator++(int) {
            Iter tmp(static_cast<Iter&>(*this));
            ++*this;
            return tmp;
        }

    protected:
        T bits_;

        template <typename U, typename I>
        friend bool operator==(const BitIteratorBase<U, I>& lhs, const BitIteratorBase<U, I>& rhs);
    };

    template <typename T, typename Iter>
    bool operator==(const BitIteratorBase<T, Iter>& lhs, const BitIteratorBase<T, Iter>& rhs) {
        return lhs.bits_ == rhs.bits_;
    }

    template <typename T, typename Iter>
    bool operator!=(const BitIteratorBase<T, Iter>& lhs, const BitIteratorBase<T, Iter>& rhs) {
        return !(lhs == rhs);
    }

    template <typename T>
    class LowToHighBitIterator : public BitIteratorBase<T, LowToHighBitIterator<T>> {
    public:
        using BitIteratorBase<T, LowToHighBitIterator<T>>::BitIteratorBase;

        uint32_t operator*() const {
            DCHECK_NE(this->bits_, 0u);
            return CTZ(this->bits_);
        }
    };

    template <typename T>
    class HighToLowBitIterator : public BitIteratorBase<T, HighToLowBitIterator<T>> {
    public:
        using BitIteratorBase<T, HighToLowBitIterator<T>>::BitIteratorBase;

        uint32_t operator*() const {
            DCHECK_NE(this->bits_, 0u);
            static_assert(std::numeric_limits<T>::radix == 2, "Unexpected radix!");
            return std::numeric_limits<T>::digits - 1u - CLZ(this->bits_);
        }
    };

    template <typename T>
    IterationRange<LowToHighBitIterator<T>> LowToHighBits(T bits) {
        return IterationRange<LowToHighBitIterator<T>>(
                LowToHighBitIterator<T>(bits), LowToHighBitIterator<T>());
    }

    template <typename T>
    IterationRange<HighToLowBitIterator<T>> HighToLowBits(T bits) {
        return IterationRange<HighToLowBitIterator<T>>(
                HighToLowBitIterator<T>(bits), HighToLowBitIterator<T>());
    }
    
}
#endif //MANXI_CORE_BIT_UTILS_H
