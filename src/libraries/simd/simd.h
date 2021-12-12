#pragma once

/* The Intel-style _mm_add_ps() and __m128 syntax is completely arbitrary in
 * GCC. GNU implemented it with wrapper libraries around their own, arguably
 * more reasonable, compiler intrinsics that already understand arithmetic
 * operators. Then, authors of SIMD-wrapper libraries will wrap *those* wrappers
 * to put arithmetic operators back on top with yet more types and functions!
 *
 * To streamline this, libCat uses the tools that GNU already provides. */

#include <type_traits>

namespace std::detail {

// TODO: Understand alignment better.
template <typename T, usize Width>
struct alignas(32) simd_vector {
    using type = T;
    static constexpr usize width = Width;
    // vector_size is a GCC attribute that represents SIMD data-types.
    T value __attribute__((vector_size(sizeof(T) * Width)));

    auto operator+(simd_vector<T, Width> const& operand)
        -> simd_vector<T, Width> {
        return simd_vector<T, Width>{this->value + operand.value};
    }
    auto operator+=(simd_vector<T, Width> const& operand)
        -> simd_vector<T, Width>& {
        this->value = this->value + operand.value;
        return *this;
    }

    auto operator-(simd_vector<T, Width> const& operand)
        -> simd_vector<T, Width> {
        return simd_vector<T, Width>{this->value - operand.value};
    }
    auto operator-=(simd_vector<T, Width> const& operand)
        -> simd_vector<T, Width>& {
        this->value = this->value - operand.value;
        return *this;
    }
};

}  // namespace std::detail

// Vectors of up to 32 8-bit integers are supported by AVX2.
using i8x2 = std::detail::simd_vector<i8, 2>;
using i8x4 = std::detail::simd_vector<i8, 4>;
using i8x8 = std::detail::simd_vector<i8, 8>;
using i8x16 = std::detail::simd_vector<i8, 16>;
using i8x32 = std::detail::simd_vector<i8, 32>;
using u8x2 = std::detail::simd_vector<u8, 2>;
using u8x4 = std::detail::simd_vector<u8, 4>;
using u8x8 = std::detail::simd_vector<u8, 8>;
using u8x16 = std::detail::simd_vector<char, 16>;
using u8x32 = std::detail::simd_vector<u8, 32>;

// Vectors of up to 16 16-bit integers are supported by AVX2.
using i16x2 = std::detail::simd_vector<i16, 2>;
using i16x4 = std::detail::simd_vector<i16, 4>;
using i16x16 = std::detail::simd_vector<i16, 16>;
using u16x2 = std::detail::simd_vector<u16, 2>;
using u16x3 = std::detail::simd_vector<u16, 3>;
using u16x4 = std::detail::simd_vector<u16, 4>;

// Vectors of up to 8 32-bit integers are supported by AVX2.
using i32x2 = std::detail::simd_vector<i32, 2>;
using i32x4 = std::detail::simd_vector<i32, 4>;
using i32x8 = std::detail::simd_vector<i32, 8>;
using u32x2 = std::detail::simd_vector<u32, 2>;
using u32x4 = std::detail::simd_vector<u32, 4>;
using u32x8 = std::detail::simd_vector<u32, 8>;

// Vectors of up to 8 32-bit floats are supported by AVX2.
using f32x2 = std::detail::simd_vector<f32, 2>;
using f32x4 = std::detail::simd_vector<f32, 4>;
using f32x8 = std::detail::simd_vector<f32, 8>;

// TODO: Evaluate what support for 64-bit ints exists in x86-64.
// Vectors of up to 4 64-bit floats are supported by AVX2.
// using f64x2 = std::detail::vector<f64, 2>;
// using f64x4 = std::detail::vector<f64, 4>;

// Vectors of up to 32 8-bit bools are supported by AVX2.
using bool8x2 = std::detail::simd_vector<bool8, 2>;
using bool8x4 = std::detail::simd_vector<bool8, 4>;
using bool8x8 = std::detail::simd_vector<bool8, 8>;
using bool8x16 = std::detail::simd_vector<bool8, 16>;
using bool8x32 = std::detail::simd_vector<bool8, 32>;

// Vectors of up to 16 8-bit bools are supported by AVX2.
using bool16x2 = std::detail::simd_vector<bool16, 2>;
using bool16x4 = std::detail::simd_vector<bool16, 4>;
using bool16x8 = std::detail::simd_vector<bool16, 8>;
using bool16x16 = std::detail::simd_vector<bool16, 16>;

// Vectors of up to 8 32-bit bools are supported by AVX2.
using bool32x2 = std::detail::simd_vector<bool32, 2>;
using bool32x4 = std::detail::simd_vector<bool32, 4>;
using bool32x8 = std::detail::simd_vector<bool32, 8>;

enum VectorMask : u8
{
    // Source data format.
    SIDD_UBYTE_OPS = 0x00,
    SIDD_UWORD_OPS = 0x01,
    SIDD_SBYTE_OPS = 0x02,
    SIDD_SWORD_OPS = 0x03,
    // Comparison operation.
    SIDD_CMP_EQUAL_ANY = 0x00,
    SIDD_CMP_RANGES = 0x04,
    SIDD_CMP_EQUAL_EACH = 0x08,
    SIDD_CMP_EQUAL_ORDERED = 0x0c,
    // Polarity.
    SIDD_POSITIVE_POLARITY = 0x00,
    SIDD_NEGATIVE_POLARITY = 0x10,
    SIDD_MASKED_POSITIVE_POLARITY = 0x20,
    SIDD_MASKED_NEGATIVE_POLARITY = 0x30,
    // Output selection in _mm_cmpXstri()
    SIDD_LEAST_SIGNIFICANT = 0x00,
    SIDD_MOST_SIGNIFICANT = 0x40,
    // Output selection in _mm_cmpXstrm ().
    SIDD_BIT_MASK = 0x00,
    SIDD_UNIT_MASK = 0x40,
};

// setzero_vector is analogous to the _mm_setzero_ function family.
template <typename T>
auto simd_setzero() {
    // TODO: Is there a cleverer way to do this? Variadic templates?
    using type = typename T::type;
    if constexpr (T::width == 2) {
        return std::detail::simd_vector<type, T::width>{0, 0};
    }
    if constexpr (T::width == 4) {
        return std::detail::simd_vector<type, T::width>{0, 0, 0, 0};
    }
    if constexpr (T::width == 8) {
        return std::detail::simd_vector<type, T::width>{0, 0, 0, 0, 0, 0, 0, 0};
    }
    if constexpr (T::width == 16) {
        return std::detail::simd_vector<type, T::width>{0, 0, 0, 0, 0, 0, 0,
                                                        0, 0, 0, 0, 0, 0, 0};
    }
    if constexpr (T::width == 32) {
        return std::detail::simd_vector<type, T::width>{
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        };
    }
}

// This is analogous to the _mm_load_ function family.
auto simd_load(auto const* in_vector) {
    return *in_vector;
}

template <u8 Mask>
auto simd_cmp_implicit_str_c(auto vector_1, auto vector_2) -> bool {
    static_assert(std::is_same_v<decltype(vector_1), decltype(vector_2)>);
    if constexpr (std::is_same_v<decltype(vector_1), u8x16>) {
        return __builtin_ia32_pcmpistric128(vector_1.value, vector_2.value,
                                            Mask);
    }
}

template <u8 Mask>
auto simd_cmp_implicit_str_i(auto vector_1, auto vector_2) -> i32 {
    static_assert(std::is_same_v<decltype(vector_1), decltype(vector_2)>);
    if constexpr (std::is_same_v<decltype(vector_1), u8x16>) {
        return __builtin_ia32_pcmpistri128(vector_1.value, vector_2.value,
                                           Mask);
    }
}

// TODO: __builtin_cpu_init() may need to be extracted out.
auto is_mmx_supported() -> bool {
    __builtin_cpu_init();
    return __builtin_cpu_supports("mmx");
}

auto is_sse1_supported() -> bool {
    __builtin_cpu_init();
    return __builtin_cpu_supports("sse");
}

auto is_sse2_supported() -> bool {
    __builtin_cpu_init();
    return __builtin_cpu_supports("sse2");
}

auto is_sse3_supported() -> bool {
    __builtin_cpu_init();
    return __builtin_cpu_supports("sse3");
}

auto is_ssse3_supported() -> bool {
    __builtin_cpu_init();
    return __builtin_cpu_supports("ssse3");
}

auto is_sse4_1_supported() -> bool {
    __builtin_cpu_init();
    return __builtin_cpu_supports("sse4.1");
}

auto is_sse4_2_supported() -> bool {
    __builtin_cpu_init();
    return __builtin_cpu_supports("sse4.2");
}

auto is_avx_supported() -> bool {
    __builtin_cpu_init();
    return __builtin_cpu_supports("avx");
}

auto is_avx2_supported() -> bool {
    __builtin_cpu_init();
    return __builtin_cpu_supports("avx2");
}

auto is_avx512f_supported() -> bool {
    __builtin_cpu_init();
    return __builtin_cpu_supports("avx512f");
}

auto is_avx512vl_supported() -> bool {
    __builtin_cpu_init();
    return __builtin_cpu_supports("avx512vl");
}