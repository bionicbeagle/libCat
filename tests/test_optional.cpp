#include <cat/memory>
#include <cat/optional>
#include <cat/unique>
#include <cat/utility>

struct Movable {
    Movable() = default;
    Movable(Movable&&) = default;
    auto operator=(Movable&&){};
};

int4 global_int = 0;
struct NonTrivial {
    int4 data;
    NonTrivial() {
        this->data = 1;
        ++global_int;
    }
    NonTrivial(NonTrivial const& in) : data(in.data + 2) {
        ++global_int;
    }
    // This gets called once, making `data` == 2.
    NonTrivial(NonTrivial&& in) : data(in.data + 1) {
        ++global_int;
    }
    ~NonTrivial() {
        this->data = 0;
        ++global_int;
    }
    NonTrivial(int, int, char) {
        this->data = 2;
    }
};

struct ConstNonTrivial {
    int4 const data;
    constexpr ConstNonTrivial() : data(1) {
    }
    constexpr ConstNonTrivial(ConstNonTrivial const& in) : data(in.data + 2) {
    }
    constexpr ConstNonTrivial(ConstNonTrivial&& in) : data(in.data + 1) {
    }
};

auto main() -> int {
    // Initialize empty.
    cat::Optional<int4> foo(nullopt);
    verify(!foo.has_value());

    cat::Optional<int4> inplace_1{};
    verify(!inplace_1.has_value());

    // `int4` default-initializes to 0.
    cat::Optional<int4> inplace_2{in_place};
    verify(inplace_2.value() == 0);

    // Assign a value.
    foo = 1;
    verify(foo.has_value());

    // Remove a value.
    foo = nullopt;
    verify(!foo.has_value());

    // Unwrap a value.
    cat::Optional<int4> moo = 1;
    moo = 2;
    verify(moo.value() == 2);

    moo = nullopt;
    verify(moo.value_or(100) == 100);

    // `Optional` reference.
    cat::Optional<int4&> ref(nullopt);
    cat::Optional<int4&> ref_2 = nullopt;

    verify(!ref.has_value());
    verify(!ref_2.has_value());

    // Rebind.
    int4 goo = 1;
    cat::Optional<int4&> boo = goo;
    ref = boo;
    boo = nullopt;

    // Because `boo` was rebinded when assigned `nullopt`, `ref` should still
    // hold a value.
    verify(ref.has_value());

    verify(ref.value() == 1);
    goo = 2;
    verify(ref.value() == 2);

    int4 goo_2 = 3;
    // `ref` is rebinded to `goo_2`, instead of `3` assigning through into
    // `goo`.
    ref = goo_2;
    verify(goo == 2);
    goo = 0;
    verify(ref.value() == 3);

    ref = nullopt;
    verify(!ref.has_value());

    ref_2 = goo;
    verify(ref_2.has_value());
    verify(ref_2.value() == goo);

    // `Optional` with a predicate.
    cat::Optional<cat::Compact<int4,
                               [](int4 input) -> bool {
                                   return input >= 0;
                               },
                               -1>>
        positive(nullopt);
    verify(!positive.has_value());

    positive = -10;
    verify(!positive.has_value());

    positive = 0;
    verify(positive.has_value());
    _ = positive.or_exit();

    positive = 10;
    verify(positive.has_value());

    positive = nullopt;
    verify(!positive.has_value());

    // `Optional<void>` with a predicate.
    cat::Optional<cat::Compact<cat::MonostateStorage<int, 0>,
                               [](int input) -> bool {
                                   return input >= 0;
                               },
                               -1>>
        predicate_void(nullopt);
    verify(!predicate_void.has_value());
    predicate_void = monostate;
    verify(predicate_void.has_value());
    _ = predicate_void.or_exit();

    // Test the sentinel predicate.
    cat::Optional<cat::Sentinel<int4, 0>> nonzero = nullopt;
    verify(!nonzero.has_value());

    nonzero = 1;
    verify(nonzero.has_value());

    nonzero = 0;
    verify(!nonzero.has_value());

    // Test `OptionalPtr`.
    int4 get_addr = 0;
    cat::OptionalPtr<int4> opt_ptr = &get_addr;
    verify(opt_ptr.has_value());
    verify(opt_ptr.value() == &get_addr);
    verify(*opt_ptr.value() == 0);
    verify(opt_ptr.p_value() == &get_addr);

    opt_ptr = nullopt;
    verify(!opt_ptr.has_value());
    opt_ptr = nullptr;
    verify(!opt_ptr.has_value());

    // Converting assignments. `foo` is `int4`.
    foo = int{1};
    foo = short{2};

    // Monadic methods.
    moo = 2;

    // Type converting transform.
    verify(moo.transform([](int4 input) -> uint8 {
                  return static_cast<uint8>(input * 2);
              })
               .value() == 4u);

    moo.or_else([] {
        cat::exit(1);
    });

    moo = nullopt;
    verify(!moo.transform([](int4 input) {
                   return input * 2;
               })
                .has_value());

    _ = moo.and_then([](int4 input) -> cat::Optional<int4> {
        cat::exit(1);
        return input;
    });

    verify(!moo.transform([](int4 input) {
                   return input * 2;
               })
                .and_then([](int4 input) -> cat::Optional<int4> {
                    return input;
                })
                .has_value());

    positive = nullopt;
    verify(!positive
                .transform([](int4 input) -> int4 {
                    return input * 2;
                })
                .has_value());

    verify(!positive
                .transform([](int4 input) {
                    return input * 2;
                })
                .and_then([](int4 input) -> cat::Optional<int4> {
                    return input;
                })
                .has_value());

    decltype(positive) default_predicate_1{};
    verify(!default_predicate_1.has_value());

    // Test function calls.
    auto return_int = [](int4 input) -> int4 {
        return input + 1;
    };
    auto return_none = [](int4) -> cat::Optional<int4> {
        return nullopt;
    };
    auto return_opt = [](int4 input) -> cat::Optional<int4> {
        return input;
    };
    auto return_void = [](int4) -> void {
    };
    auto return_opt_void = [](int4) -> cat::Optional<void> {
        return monostate;
    };
    auto nothing = []() -> void {
    };
    auto maybe_nothing = []() -> cat::Optional<void> {
        return nullopt;
    };

    foo.transform(return_int).and_then(return_opt_void).or_else(nothing);

    _ = foo.transform(return_int)
            .and_then(return_opt_void)
            .or_else(maybe_nothing);

    cat::Optional<int4> monadic_int;
    monadic_int = return_none(0).and_then(return_opt);
    verify(!monadic_int.has_value());

    monadic_int = return_opt(1).transform(return_int);
    verify(monadic_int.has_value());
    verify(monadic_int.value() == 2);

    cat::Optional<void> monadic_void =
        return_opt(1).transform(return_int).transform(return_void);
    verify(monadic_void.has_value());

    // Test monadic methods on reference types.
    int4 monadic_int_ref = 1;
    cat::Optional<void> monadic_void_ref =
        cat::Optional{monadic_int_ref}.and_then(return_opt_void);
    // Be sure that this did not assign through.
    verify(monadic_void_ref.has_value());

    // The default value of `int4` is `0`.
    decltype(positive) default_predicate_2{in_place};
    verify(default_predicate_2.value() == 0);

    // Test monadic methods on move-only types.
    cat::Optional<cat::Unique<int4>> monadic_move = 1;
    monadic_move = return_none(0).and_then(return_opt);
    verify(!monadic_move.has_value());

    monadic_move = return_opt(1).transform(return_int);
    verify(monadic_move.has_value());
    verify(monadic_move.value().borrow() == 2);

    // Test copying `Optional`s into other `Optional`s.
    cat::Optional<int4> opt_original = 10;
    cat::Optional<int4> opt_copy_1 = cat::Optional{opt_original};
    cat::Optional<int4> opt_copy_2 = opt_original;
    verify(opt_copy_1.value() == 10);
    verify(opt_copy_2.value() == 10);

    // Getting pointers.
    foo = 1;
    int4 const& ref_foo = foo.value();
    verify(&ref_foo == &foo.value());
    verify(foo.p_value() == &foo.value());
    verify(foo.p_value() == addressof(foo.value()));

    // Test non-trivial reference.
    NonTrivial nontrivial_val;
    cat::Optional<NonTrivial&> nontrivial_ref_default;
    nontrivial_ref_default = nontrivial_val;
    [[maybe_unused]] cat::Optional<NonTrivial&> nontrivial_ref = nontrivial_val;

    NonTrivial const const_nontrivial_val;
    [[maybe_unused]] cat::Optional<NonTrivial&> const
        mut_const_nontrivial_ref_default;
    [[maybe_unused]] cat::Optional<NonTrivial&> const mut_const_nontrivial_ref =
        nontrivial_val;

    [[maybe_unused]] cat::Optional<NonTrivial const&>
        const_mut_nontrivial_ref_default;
    [[maybe_unused]] cat::Optional<NonTrivial const&> const_mut_nontrivial_ref =
        nontrivial_val;
    [[maybe_unused]] cat::Optional<NonTrivial const&>
        const_mut_nontrivial_ref_2 = const_nontrivial_val;
    const_mut_nontrivial_ref = const_nontrivial_val;

    [[maybe_unused]] cat::Optional<NonTrivial const&> const
        const_const_nontrivial_ref_default;
    [[maybe_unused]] cat::Optional<NonTrivial const&> const
        const_const_nontrivial_ref = nontrivial_val;
    [[maybe_unused]] cat::Optional<NonTrivial const&> const
        const_const_nontrivial_ref_2 = const_nontrivial_val;

    // `Optional const`
    cat::Optional<int4> const constant_val = 1;
    [[maybe_unused]] cat::Optional<int4> const constant_null = nullopt;
    [[maybe_unused]] auto con = constant_val.value();

    // Test constant references.
    int4 nonconstant_int = 10;
    int4 const constant_int = 9;
    cat::Optional<int4 const&> constant_ref = constant_int;
    verify(constant_ref.value() == 9);
    constant_ref = nonconstant_int;
    verify(constant_ref.value() == 10);

    // Test move-only types.
    Movable mov;
    cat::Optional<Movable> maybe_movs(cat::move(mov));

    // Non-trivial constructor and destructor.
    cat::Optional<NonTrivial> nontrivial = NonTrivial();
    verify(nontrivial.value().data == 2);

    // `Optional<void>` default-initializes empty:
    cat::Optional<void> optvoid;
    verify(!optvoid.has_value());
    // `monostate` initializes a value:
    cat::Optional<void> optvoid_2{monostate};
    verify(optvoid_2.has_value());

    // `in_place` initializes a value:
    cat::Optional<void> optvoid_4{in_place};
    verify(optvoid_4.has_value());
    // `nullopt` initializes empty:
    cat::Optional<void> optvoid_5{nullopt};
    verify(!optvoid_5.has_value());

    cat::Optional<NonTrivial> in_place_nontrivial_1{in_place};
    verify(in_place_nontrivial_1.has_value());
    verify(in_place_nontrivial_1.value().data == 1);

    cat::Optional<NonTrivial> in_place_nontrivial_2{in_place, 1, 2, 'a'};
    verify(in_place_nontrivial_2.has_value());
    verify(in_place_nontrivial_2.value().data == 2);

    // Test `Optional` in a `constexpr` context.
    auto constant = []() constexpr {
        [[maybe_unused]] constexpr cat::Optional<int> const_int_default;

        constexpr cat::Optional<ConstNonTrivial> const_nontrivial_default;
        verify(!const_nontrivial_default.has_value());

        constexpr cat::Optional<ConstNonTrivial> const_nontrivial =
            ConstNonTrivial{};
        verify(const_nontrivial.has_value());

        constexpr cat::Optional<ConstNonTrivial> const_nontrivial_in_place = {
            in_place, ConstNonTrivial{}};
        verify(const_nontrivial_in_place.has_value());

        // Test `Optional<Compact<T>>`.
        constexpr cat::OptionalPtr<void> const_optptr = nullptr;
        cat::OptionalPtr<void> optptr = nullptr;
        optptr = nullptr;
        optptr = const_optptr;
        [[maybe_unused]] cat::OptionalPtr<void> optptr2 = optptr;
        [[maybe_unused]] cat::OptionalPtr<void> optptr3 = const_optptr;
        [[maybe_unused]] cat::OptionalPtr<void> optptr4;

        [[maybe_unused]] constexpr cat::OptionalPtr<NonTrivial>
            const_nontrivial_optptr = nullptr;
        [[maybe_unused]] constexpr cat::OptionalPtr<NonTrivial>
            const_nontrivial_default_optptr;
        [[maybe_unused]] cat::OptionalPtr<NonTrivial> nontrivial_optptr =
            nullptr;
        [[maybe_unused]] cat::OptionalPtr<NonTrivial> nontrivial_default_optptr;
    };
    _ = cat::constant_evaluate(constant);

    // Assign value:
    optvoid = monostate;
    verify(optvoid.has_value());
    // Remove value:
    optvoid = nullopt;
    verify(!optvoid.has_value());

    // Test `.is()`;
    cat::Optional<int4> opt_is = 1;
    verify(opt_is.is<int4>());
    verify(!opt_is.is<uint8>());

    opt_is = nullopt;
    verify(!opt_is.is<int4>());
    verify(!opt_is.is<uint8>());

    // Test `match()`.
    cat::Optional<int4> opt_match = 1;

    bool matched = false;
    cat::match(opt_match)(is_a(1).then([&]() {
        matched = true;
    }));
    cat::match(opt_match)(is_a(2).then([&]() {
        matched = false;
    }));
    verify(matched);

    matched = false;
    cat::match(opt_match)(is_a<int4>().then([&]() {
        matched = true;
    }));
    cat::match(opt_match)(is_a<uint8>().then([&]() {
        matched = false;
    }));
    verify(matched);

    // Test matching against `nullopt` when this holds a value.
    matched = true;
    cat::match(opt_match)(is_a(nullopt).then([&]() {
        matched = false;
    }));
    verify(matched);

    // Test matching against `nullopt` when this does not hold a value.
    matched = false;
    opt_match = nullopt;
    cat::match(opt_match)(is_a(nullopt).then([&]() {
        matched = true;
    }));
    verify(matched);

    // Test member access pattern matching syntax.
    matched = false;
    opt_match.match(is_a(nullopt).then([&]() {
        matched = true;
    }));
    verify(matched);
}
