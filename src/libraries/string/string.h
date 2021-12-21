// -*- mode: c++ -*-
// vim: set ft=cpp:
#pragma once

#include <stdint.h>

// TODO: Optimize string_length().
// https://newbedev.com/why-does-glibc-s-strlen-need-to-be-so-complicated-to-run-quickly
// https://git.musl-libc.org/cgit/musl/tree/src/string/strlen.c

namespace std {

/* T is the return type of string_length(). It may be signed or unsigned. */
template <typename T>
constexpr auto string_length_as(char8_t const* p_string) -> T {
    T result = 0;
    while (p_string[result] != '\0') {
        result++;
    }
    return result;
}

// TODO: This should use an AnyConst container.
void copy_memory(void const* p_source, void* p_destination, isize bytes) {
    char const* p_source_handle = static_cast<char const*>(p_source);
    char* p_destination_handle = static_cast<char*>(p_destination);

    for (isize i = 0; i < bytes; i++) {
        p_destination_handle[i] = p_source_handle[i];
    }
}

// TODO: Move into a <bit.h library.
auto is_aligned(void const volatile* pointer, isize byte_alignment) -> bool {
    return (reinterpret_cast<usize>(pointer) % byte_alignment) == 0;
}

}  // namespace std

using std::copy_memory;
using std::is_aligned;
using std::string_length_as;

namespace simd {

// TODO: Power-of-two concept.
// template <isize Width>
// void copy_memory_small(void* p_destination, void const* p_source) {
//     static_assert(Width <= 256);
//     using Vector = std::detail::simd_vector<i1, Width>;
//     Vector source_vector = *reinterpret_cast<Vector const*>(p_source);
//     *reinterpret_cast<Vector*>(p_destination) = source_vector;
// }

// TODO: Use Any instead of chars.
void copy_memory(void const* p_source, void* p_destination, isize bytes) {
    // Vector is the width of a 32-byte AVX register.
    // `long long int` is required for some SIMD intrinsics.
    using Vector = std::detail::simd_vector<long long int, 4>;

    unsigned char const* p_source_handle =
        reinterpret_cast<unsigned char const*>(p_source);
    unsigned char* p_destination_handle =
        reinterpret_cast<unsigned char*>(p_destination);
    static isize cachesize = 0x200000;  // L3-cache size.
    isize padding;

    if (bytes <= 256) {
        std::copy_memory(p_source, p_destination, bytes);
    }

    // Align src, dst, and size to 16 bytes
    padding =
        (32 - ((reinterpret_cast<isize>(p_destination_handle)) & 31)) & 31;

    Vector head = *static_cast<Vector const*>(p_source);
    *static_cast<Vector*>(p_destination) = head;
    p_source_handle += padding;
    p_destination_handle += padding;
    bytes -= padding;

    Vector vectors[8];
    // This routine is optimized for buffers in L3 cache. Streaming is slower.
    if (bytes <= cachesize) {
        while (bytes >= 256) {
            // TODO: Unroll the two loops in this scope?
            for (i4 i = 0; i < 8; i++) {
                vectors[i] =
                    *(const_cast<Vector*>(
                          reinterpret_cast<Vector const*>(p_source_handle)) +
                      i);
            }
            prefetch(p_source_handle + 512, simd::MM_HINT_NTA);
            p_source_handle += 256;
            for (i4 i = 0; i < 8; i++) {
                *(reinterpret_cast<Vector*>(p_destination_handle)) = vectors[i];
            }
            p_destination_handle += 256;
            bytes -= 256;
        }
    } else {
        prefetch(p_source_handle + 512, simd::MM_HINT_NTA);
        /* TODO: This could be improved by using aligned-streaming when
         * possible. */
        while (bytes >= 256) {
            // TODO: Unroll the two loops in this scope?
            for (i4 i = 0; i < 8; i++) {
                vectors[i] =
                    *(const_cast<Vector*>(
                          reinterpret_cast<Vector const*>(p_source_handle)) +
                      i);
            }
            prefetch(p_source_handle + 512, simd::MM_HINT_NTA);
            p_source_handle += 256;
            for (i4 i = 0; i < 8; i++) {
                stream_in(p_destination_handle, &vectors[i]);
            }
            p_destination_handle += 256;
            bytes -= 256;
        }
        simd::fence();
    }
    simd::zero_upper_avx_registers();
}

/* T is the return type of string_length_as(). It may be signed or
 * unsigned. This function requires SSE4.2 */
template <typename T>
auto string_length_as(char8_t const* p_string) -> T {
    // TODO: Align pointers.
    T result = 0;
    charx16* p_memory = p_string_to_p_vector<16>(p_string);
    constexpr charx16 zeroes = simd::set_zeros<charx16>();

    while (true) {
        charx16 data;
        data = *p_memory;
        constexpr u1 mask =
            SIDD_UBYTE_OPS | SIDD_CMP_EQUAL_EACH | SIDD_LEAST_SIGNIFICANT;
        if (simd::cmp_implicit_str_c<mask>(data, zeroes)) {
            i1 const index = simd::cmp_implicit_str_i<mask>(data, zeroes);
            return result + index;
        }
        p_memory++;
        result += sizeof(u1x16);
        return result;
    }
    // This point unreachable because the function would segfault first.
    __builtin_unreachable();
}

}  // namespace simd

[[deprecated("memcpy() is deprecated! Use simd::copy_buffer() instead!")]] auto
memcpy(void* p_destination, void const* p_source, size_t bytes) -> void* {
    simd::copy_memory(p_source, p_destination, bytes);
    return p_destination;
}

[[deprecated(
    "strlen() is deprecated! Use simd::string_length<T>() instead.")]] auto
strlen(char8_t const* p_string) -> size_t {
    return simd::string_length_as<size_t>(p_string);
}
