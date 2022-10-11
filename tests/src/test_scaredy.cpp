#include <cat/scaredy>

#include "../unit_tests.hpp"

// Minimal result types usable for `cat::Scaredy`.
struct ErrorOne {
    int4 code;

    constexpr auto error() const -> int8 {
        return this->code;
    }
};

struct ErrorTwo {
    int4 code;

    constexpr auto error() const -> int8 {
        return this->code;
    }
};

auto one() -> ErrorOne {
    ErrorOne one{1};
    return one;
}

auto two() -> ErrorTwo {
    ErrorTwo two{2};
    return two;
}

auto union_errors(int4 error) -> cat::Scaredy<int8, ErrorOne, ErrorTwo> {
    switch (error.raw) {
        case 0:
            return one();
        case 1:
            return two();
        case 2:
            return int8{3};
        case 3:
            return 3;
        default:
            __builtin_unreachable();
    }
}

enum class Err {
    one,
    two
};

auto scaredy_try_success() -> cat::Scaredy<int, Err> {
    cat::Scaredy<int, Err> error{0};
    int boo = TRY(error);
    return boo;
}

auto scaredy_try_fail() -> cat::Scaredy<int, Err> {
    cat::Scaredy<int, Err> error{Err::one};
    int boo = TRY(error);
    return boo;
}

TEST(test_scaredy) {
    cat::Scaredy result = union_errors(0);
    // The `Scaredy` here adds a flag to the `int8`, which is padded out to 16
    // bytes. No storage cost exists for the error types.
    static_assert(sizeof(result) == 16);

    cat::verify(!result.has_value());
    cat::verify(result.is<ErrorOne>());
    cat::verify(!result.is<int8>());

    result = union_errors(1);
    cat::verify(!result.has_value());
    cat::verify(result.is<ErrorTwo>());
    cat::verify(!result.is<int8>());

    result = union_errors(2);
    cat::verify(result.has_value());
    cat::verify(result.is<int8>());

    result = union_errors(3);
    cat::verify(result.has_value());
    cat::verify(result.value() == 3);
    cat::verify(result.is<int8>());

    // Test `.error()`.
    cat::Scaredy<int, ErrorOne> one_error = ErrorOne{1};
    cat::verify(one_error.error().code == 1);
    cat::verify(one_error.error<ErrorOne>().code == 1);

    cat::Scaredy<int, ErrorOne, ErrorTwo> two_error = ErrorOne{1};
    cat::verify(two_error.error<ErrorOne>().code == 1);

    // Test compact optimization.
    cat::Scaredy<cat::Compact<int4,
                              [](int4 input) {
                                  return input >= 0;
                              },
                              -1>,
                 ErrorOne>
        predicate = -1;
    // The `Scaredy` here adds no storage bloat to an `int4`.
    static_assert(sizeof(predicate) == sizeof(int4));
    cat::verify(!predicate.has_value());

    predicate = -1;
    cat::verify(!predicate.has_value());

    predicate = 0;
    cat::verify(predicate.has_value());

    predicate = 10;
    cat::verify(predicate.has_value());

    predicate = ErrorOne{-1};
    cat::verify(!predicate.has_value());

    // Test `.value_or()`.
    cat::Scaredy<int4, ErrorOne> is_error = ErrorOne{};
    cat::Scaredy<int4, ErrorOne> is_value = 2;
    cat::Scaredy<int4, ErrorOne> const const_is_error = ErrorOne{};
    cat::Scaredy<int4, ErrorOne> const const_is_value = 2;

    int4 fallback = is_error.value_or(1);
    cat::verify(fallback == 1);

    int4 no_fallback = is_value.value_or(1);
    cat::verify(no_fallback == 2);

    int4 const_fallback = const_is_error.value_or(1);
    cat::verify(const_fallback == 1);

    int4 no_const_fallback = const_is_value.value_or(1);
    cat::verify(no_const_fallback == 2);

    // Test monadic member functions on a mutable `Scaredy`.
    auto increment = [](auto input) {
        return input + 1;
    };

    cat::Scaredy<int4, ErrorOne> mut_scaredy = 1;
    _ = mut_scaredy.transform(increment).and_then(increment);

    // `.transform()` returning `void`.
    mut_scaredy.transform(increment).or_else([]() {
        return;
    });

    _ = mut_scaredy.transform(increment).or_else([]() {
        return decltype(mut_scaredy){};
    });

    // Test monadic member functions on a `const`-qualified `Scaredy`.
    cat::Scaredy<int4, ErrorOne> const const_scaredy = 1;
    _ = const_scaredy.transform(increment).and_then(increment);

    // Test `.is()` on variant `Scaredy`.
    bool matched = false;

    cat::Scaredy<int4, ErrorOne, ErrorTwo> is_variant_scaredy;
    is_variant_scaredy = 1;

    // Match it against `int4`.
    cat::match(is_variant_scaredy)(  //
        is_a<int4>().then([&]() {
            matched = true;
        }));
    cat::match(is_variant_scaredy)(  //
        is_a<ErrorOne>().then([&]() {
            matched = false;
        }));
    cat::match(is_variant_scaredy)(  //
        is_a<ErrorTwo>().then([&]() {
            matched = false;
        }));
    // `float` can never hold true here, but it should compile.
    cat::match(is_variant_scaredy)(  //
        is_a<float>().then([&]() {
            matched = false;
        }));
    cat::verify(matched);

    // Match it against `ErrorOne`.
    matched = false;
    is_variant_scaredy = ErrorOne{};
    cat::match(is_variant_scaredy)(  //
        is_a<ErrorOne>().then([&]() {
            matched = true;
        }));
    cat::match(is_variant_scaredy)(  //
        is_a<int4>().then([&]() {
            matched = false;
        }));
    cat::match(is_variant_scaredy)(  //
        is_a<ErrorTwo>().then([&]() {
            matched = false;
        }));
    cat::verify(matched);

    // Test member access pattern matching syntax.
    matched = false;
    is_variant_scaredy.match(is_a<ErrorOne>().then([&]() {
        matched = true;
    }));
    cat::verify(matched);

    // Test `.is()` on `Compact` `Scaredy`.
    predicate = 1;

    // Test type comparison.
    matched = false;
    cat::match(predicate)(  //
        is_a<ErrorOne>().then([&]() {
            cat::exit(1);
        }),
        is_a<int4>().then([&]() {
            matched = true;
        }));
    cat::verify(matched);

    matched = false;
    predicate = ErrorOne{-1};
    cat::match(predicate)(  //
        is_a<int4>().then([&]() {
            cat::exit(1);
        }),
        is_a<ErrorOne>().then([&]() {
            matched = true;
        }));
    cat::verify(matched);

    // Test `TRY` macro.
    _ = scaredy_try_success().verify();
    cat::Scaredy fail = scaredy_try_fail();
    cat::verify(!fail.has_value());
}
