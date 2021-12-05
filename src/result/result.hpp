#pragma once

#include <concepts>
#include <cstdlib>
#include <type_traits>
#include <utility>

template <typename T = void>
struct Result;

struct Error {
    i32 code;

    // TODO: Rethink Error and Result constructors.
    Error() = default;

    // This must be explicit to disambiguate returning an integer to a Result.
    Error(auto in_code) {
        code = static_cast<i32>(in_code);
    }

    template <typename T>
    operator Result<T>() const {
        return Result<T>(code);
    }

    // template <std::integral auto T> does not work for some reason.
    template <typename T>
    constexpr operator T() const
        // TODO: Make is_bool<> traits.
        requires(std::is_integral<T>() || std::is_same_v<T, bool>) {
        return code;
    }

    auto operator==(Error right_error) const -> bool {
        return this->code == right_error.code;
    }
};

template <typename T>
struct [[nodiscard]] Result {
    /* The error code must be laid out first, so that _start() can reliably
     * extract it from %rax. */
    Error code;  // Error is 32-bits large.

    /* char should be a relatively unintrusive dummy data for when this holds
     * void. Reflection TS in future C++ will provide conditional-members, which
     * would be a better solution. */
    using DataType = std::conditional_t<std::is_void_v<T>, char, T>;
    DataType data;

    bool is_ok;

    Result(Error in_code) {
        this->code = in_code;
        is_ok = false;
    }

    Result(DataType in_data) {
        this->data = in_data;
        is_ok = true;
    }

    // TODO: Pass in the exit code and error message with overloads.
    auto or_panic() -> T {
        if (is_ok) {
            if constexpr (!std::is_void_v<T>) {
                return this->data;
            }
            return;
        }
        exit(EXIT_FAILURE);
    }
    // TODO: Overload that prints an error message.
    /* auto or_panic(char const* error_message) -> T {
           if (is_ok) {
           if constexpr (!std::is_void_v<T>) {
           return this->data;
           }
           return;
           }
           // TODO: Print the error message.
           exit(EXIT_FAILURE);
           }
        */
    auto or_return(DataType const& in_data) -> DataType {
        return in_data;
    }

    // TODO: Add std::invokable concept
    auto or_do(auto callback) {
        if (!is_ok) {
            return callback();
        }
        if constexpr (!std::is_void_v<T>) {
            if constexpr (!std::is_void_v<T>) {
                return this->data;
            }
        }
    }

    // This function is intended to be the return value of its calling function.
    [[nodiscard]] auto or_propagate() -> Result<T> {
        return *this;
    }
};
