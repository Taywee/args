#include "args.hxx"
#include <cassert>
#include <iostream>
#include <limits>
#include <cmath>

/** Test suite for checked arithmetic functions
 * 
 * This test suite validates the SafeAdd and SafeMultiply helper functions
 * that prevent integer overflow in width and allocation arithmetic.
 */

// Test SafeAdd with size_t (the primary type used in Wrap)
void TestSafeAddSizeT()
{
    size_t result;

    // Test normal addition
    assert(args::SafeAdd<size_t>(5, 10, result));
    assert(result == 15);
    std::cout << "PASS: SafeAdd normal case (5 + 10 = 15)" << std::endl;

    // Test addition at limit boundary
    size_t max_val = std::numeric_limits<size_t>::max();
    assert(!args::SafeAdd<size_t>(max_val, 1, result));
    std::cout << "PASS: SafeAdd overflow detection (SIZE_MAX + 1)" << std::endl;

    // Test addition near limit
    assert(!args::SafeAdd<size_t>(max_val - 5, 10, result));
    std::cout << "PASS: SafeAdd near-overflow detection (SIZE_MAX - 5 + 10)" << std::endl;

    // Test addition with zero
    assert(args::SafeAdd<size_t>(0, 100, result));
    assert(result == 100);
    std::cout << "PASS: SafeAdd with zero (0 + 100 = 100)" << std::endl;

    // Test addition at exact boundary
    assert(args::SafeAdd<size_t>(max_val - 5, 5, result));
    assert(result == max_val);
    std::cout << "PASS: SafeAdd exact boundary (SIZE_MAX - 5 + 5 = SIZE_MAX)" << std::endl;
}

// Test SafeMultiply with size_t
void TestSafeMultiplySizeT()
{
    size_t result;

    // Test normal multiplication
    assert(args::SafeMultiply<size_t>(5, 10, result));
    assert(result == 50);
    std::cout << "PASS: SafeMultiply normal case (5 * 10 = 50)" << std::endl;

    // Test multiplication that would overflow
    size_t max_val = std::numeric_limits<size_t>::max();
    assert(!args::SafeMultiply<size_t>(max_val, 2, result));
    std::cout << "PASS: SafeMultiply overflow detection (SIZE_MAX * 2)" << std::endl;

    // Test multiplication with zero
    assert(args::SafeMultiply<size_t>(0, 1000000, result));
    assert(result == 0);
    std::cout << "PASS: SafeMultiply with zero (0 * 1000000 = 0)" << std::endl;

    // Test large multiplications that overflow (for 64-bit systems)
    // Use values that will definitely overflow
    size_t large_val = 10000000000UL;  // 10 billion
    assert(!args::SafeMultiply<size_t>(large_val, large_val, result));
    std::cout << "PASS: SafeMultiply large value overflow (10B * 10B)" << std::endl;

    // Test multiplication that doesn't overflow
    size_t safe_val = 1000000UL;  // 1 million
    assert(args::SafeMultiply<size_t>(safe_val, safe_val, result));
    assert(result == safe_val * safe_val);
    std::cout << "PASS: SafeMultiply safe value success (1M * 1M)" << std::endl;
}

// Test SafeAdd with int
void TestSafeAddInt()
{
    int result;

    // Test normal addition
    assert(args::SafeAdd<int>(100, 200, result));
    assert(result == 300);
    std::cout << "PASS: SafeAdd int normal case (100 + 200 = 300)" << std::endl;

    // Test negative number handling
    assert(args::SafeAdd<int>(-100, 50, result));
    assert(result == -50);
    std::cout << "PASS: SafeAdd int with negative (-100 + 50 = -50)" << std::endl;

    // Test negative operand underflow detection
    assert(!args::SafeAdd<int>(std::numeric_limits<int>::min(), -1, result));
    std::cout << "PASS: SafeAdd int underflow detection (INT_MIN + -1)" << std::endl;

    // Test int overflow
    int max_int = std::numeric_limits<int>::max();
    assert(!args::SafeAdd<int>(max_int, 1, result));
    std::cout << "PASS: SafeAdd int overflow detection (INT_MAX + 1)" << std::endl;
}

// Test SafeMultiply with int
void TestSafeMultiplyInt()
{
    int result;

    // Test normal multiplication
    assert(args::SafeMultiply<int>(10, 20, result));
    assert(result == 200);
    std::cout << "PASS: SafeMultiply int normal case (10 * 20 = 200)" << std::endl;

    // Test multiplication overflow
    int max_int = std::numeric_limits<int>::max();
    assert(!args::SafeMultiply<int>(max_int, 2, result));
    std::cout << "PASS: SafeMultiply int overflow detection (INT_MAX * 2)" << std::endl;

    // Test negative multiplication
    assert(args::SafeMultiply<int>(-10, 20, result));
    assert(result == -200);
    std::cout << "PASS: SafeMultiply int negative (-10 * 20 = -200)" << std::endl;
}

// Test Wrap function with boundary conditions
void TestWrapBoundaryConditions()
{
    // Test with very large width that could cause overflow
    std::vector<std::string> words = {"hello", "world", "test"};
    
    // This should not cause overflow or crash
    auto result = args::Wrap(words.begin(), words.end(), std::numeric_limits<size_t>::max());
    assert(!result.empty());
    std::cout << "PASS: Wrap with SIZE_MAX width" << std::endl;

    // Test with width of 1 (minimal width)
    result = args::Wrap(words.begin(), words.end(), 1);
    assert(!result.empty());
    std::cout << "PASS: Wrap with width = 1" << std::endl;

    // Test with width of 0 (edge case)
    result = args::Wrap(words.begin(), words.end(), 0);
    assert(!result.empty());
    std::cout << "PASS: Wrap with width = 0" << std::endl;

    // Test with empty words vector
    std::vector<std::string> empty_words;
    result = args::Wrap(empty_words.begin(), empty_words.end(), 80);
    assert(result.empty());
    std::cout << "PASS: Wrap with empty words vector" << std::endl;

    // Test with newline separator
    std::vector<std::string> words_with_newline = {"hello", "\n", "world"};
    result = args::Wrap(words_with_newline.begin(), words_with_newline.end(), 80);
    assert(result.size() >= 2);
    std::cout << "PASS: Wrap with newline separator" << std::endl;
}

// Test Wrap function with large string input
void TestWrapLargeStringInput()
{
    // Test with very long string that could trigger width calculation issues
    std::string long_string(1000, 'a');
    
    auto result = args::Wrap(long_string, 80);
    assert(!result.empty());
    std::cout << "PASS: Wrap long string (1000 chars) with width 80" << std::endl;

    // Test with extremely large width
    result = args::Wrap(long_string, std::numeric_limits<size_t>::max());
    assert(result.size() >= 1);
    std::cout << "PASS: Wrap long string with SIZE_MAX width" << std::endl;

    // Test with width of 1
    result = args::Wrap(long_string, 1);
    assert(!result.empty());
    std::cout << "PASS: Wrap long string with width 1" << std::endl;
}

// Main test runner
int main()
{
    std::cout << "Starting Checked Arithmetic Test Suite\n" << std::endl;

    std::cout << "=== SafeAdd Tests ===" << std::endl;
    TestSafeAddSizeT();
    TestSafeAddInt();

    std::cout << "\n=== SafeMultiply Tests ===" << std::endl;
    TestSafeMultiplySizeT();
    TestSafeMultiplyInt();

    std::cout << "\n=== Wrap Function Boundary Tests ===" << std::endl;
    TestWrapBoundaryConditions();
    TestWrapLargeStringInput();

    std::cout << "\n=== All Tests Passed ===" << std::endl;
    return 0;
}