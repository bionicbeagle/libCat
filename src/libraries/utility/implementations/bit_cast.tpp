// -*- mode: c++ -*-
// vim: set ft=cpp:
#pragma once

#include <meta>
#include <utility>

/* As far as I can prove, `bit_cast` is a zero-overhead abstraction on `-O3`.
 * It copies data at some memory byte-by-byte into a differently-typed variable
 * at its own address. Compilers are good at optimizing out this pattern. Four
 * overloads are required, which I believe is still simpler than metaprogramming
 * one `bit_cast` function. A `void*`, `void* const`, `T`, and `T const` each
 * resolve to different overloads. These are optimized and inlined because it is
 * sometimes semantically necessary that these have zero-overhead, and those
 * attributes guarantee this even in -O0. */

// TODO: Add an overload for C++20 bit casting.

// clangd emits diagnistic false positives in this file.

template <typename T>
[[gnu::warning(
    "This function is intended to be inlined and constant-folded \
out.")]] [[gnu::optimize("-O3")]] [[gnu::always_inline]] constexpr inline auto
meta::bit_cast(auto& from_value) -> T
    // If the value is not `const`:
    requires(!meta::is_const<meta::RemoveReference<decltype(from_value)>>) {
    char* p_from = static_cast<char*>(static_cast<void*>(&from_value));

    T* p_to = static_cast<T*>(static_cast<void*>(p_from));
    // GCC optimizes this pattern into a bit-cast:
    for (unsigned i = 0; i < sizeof(T); i++) {
        static_cast<char*>(static_cast<void*>(p_to))[i] = p_from[i];
    }
    return *p_to;
}

template <typename T>
[[gnu::warning(
    "This function is intended to be inlined and constant-folded \
out.")]] [[gnu::optimize("-O3")]] [[gnu::always_inline]] constexpr inline auto
meta::bit_cast(auto& from_value) -> T
    // If the value is `const`:
    requires(meta::is_const<meta::RemoveReference<decltype(from_value)>>) {
    /* Cast the address of `from_value` into a pointer of its type (with the
     * reference removed), then remove its `const` qualifier. Cast that to
     * `void*`, then cast that to `char*`, which represents bytes. */
    char* p_from = static_cast<char*>(static_cast<void*>(
        const_cast<
            meta::RemoveConst<meta::RemoveReference<decltype(from_value)>>*>(
            &from_value)));

    T* p_to = static_cast<T*>(static_cast<void*>(p_from));
    // GCC optimizes this pattern into a bit-cast:
    for (unsigned i = 0; i < sizeof(T); i++) {
        static_cast<char*>(static_cast<void*>(p_to))[i] = p_from[i];
    }
    return *p_to;
}
