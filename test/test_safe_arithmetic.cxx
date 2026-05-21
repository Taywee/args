// Regression tests for checked arithmetic helpers
// Compile with: g++ -std=c++11 -o test_safe_arithmetic test_safe_arithmetic.cpp

#include "safe_arithmetic.hxx"
#include <cassert>
#include <limits>
#include <iostream>

using namespace args;

int main() {
    unsigned int out;
    // SafeMul tests
    assert(SafeMul(10u, 20u, &out) && out == 200u);
    assert(!SafeMul(std::numeric_limits<unsigned int>::max(), 2u, &out));
    assert(!SafeMul(std::numeric_limits<unsigned int>::max(), std::numeric_limits<unsigned int>::max(), &out));
    assert(SafeMul(0u, 100u, &out) && out == 0u);

    // SafeAdd tests
    assert(SafeAdd(1u, 2u, &out) && out == 3u);
    assert(!SafeAdd(std::numeric_limits<unsigned int>::max(), 1u, &out));
    assert(SafeAdd(0u, 0u, &out) && out == 0u);

    // SafeSub tests
    assert(SafeSub(5u, 3u, &out) && out == 2u);
    assert(!SafeSub(3u, 5u, &out));
    assert(SafeSub(0u, 0u, &out) && out == 0u);

    // SafeCast tests
    unsigned short out16;
    assert(SafeCast<unsigned short>(100u, &out16) && out16 == 100u);
    assert(!SafeCast<unsigned short>(std::numeric_limits<unsigned int>::max(), &out16));

#ifdef NDEBUG
    (void)out;
    (void)out16;
#endif

    std::cout << "All safe_arithmetic tests passed.\n";
    return 0;
}