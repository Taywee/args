// Centralized checked arithmetic helpers for safe allocation and arithmetic
// Systematic hardening against integer overflows and undersized allocations
// Usage: if (!SafeMul(a, b, &out)) { /* handle overflow */ }

#ifndef ARGS_SAFE_ARITHMETIC_HXX
#define ARGS_SAFE_ARITHMETIC_HXX

#include <limits>
#include <type_traits>

namespace args {

// Safe multiplication: returns false on overflow, true on success
// out is only set if multiplication succeeds
// T must be unsigned integral type

template <typename T>
bool SafeMul(T a, T b, T* out) {
    static_assert(std::is_integral<T>::value && std::is_unsigned<T>::value, "SafeMul requires unsigned integral type");
    if (b != 0 && a > std::numeric_limits<T>::max() / b) {
        return false;
    }
    *out = a * b;
    return true;
}

// Safe addition: returns false on overflow, true on success
template <typename T>
bool SafeAdd(T a, T b, T* out) {
    static_assert(std::is_integral<T>::value && std::is_unsigned<T>::value, "SafeAdd requires unsigned integral type");
    if (a > std::numeric_limits<T>::max() - b) {
        return false;
    }
    *out = a + b;
    return true;
}

// Safe subtraction: returns false on underflow, true on success
template <typename T>
bool SafeSub(T a, T b, T* out) {
    static_assert(std::is_integral<T>::value && std::is_unsigned<T>::value, "SafeSub requires unsigned integral type");
    if (a < b) {
        return false;
    }
    *out = a - b;
    return true;
}

// Safe cast: returns false if value cannot be represented in To type
template <typename To, typename From>
bool SafeCast(From value, To* out) {
    if (value < std::numeric_limits<To>::min() || value > std::numeric_limits<To>::max()) {
        return false;
    }
    *out = static_cast<To>(value);
    return true;
}

} // namespace args

#endif // ARGS_SAFE_ARITHMETIC_HXX