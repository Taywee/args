// Regression test: the integer reader must not silently accept data that
// follows an embedded NUL byte in the value string.
//
// ValueReader::ParseNumericValue for integer types parses value.c_str() with
// std::strtoull/strtoll, which treat the buffer as a C string and stop at the
// first '\0'. The "no trailing data" guard used to test `*end == '\0'`, which
// an embedded NUL satisfies prematurely: "12\0garbage" was accepted as 12 and
// everything past the NUL was silently discarded (an input-validation bypass,
// CWE-158). The istringstream-based reader used for floating point already
// rejected such input, so the two paths disagreed. The guard now validates
// against the string's true length, so both paths reject trailing data
// regardless of embedded NULs.
//
// This test FAILS on the unfixed library (the integer cases parse as success)
// and passes once the length-based check is in place.

#include "test_common.hxx"
#include <args.hxx>
#include "test_helpers.hxx"
#include <iostream>
#include <string>

template <typename T>
static void expect_reject(const std::string &input)
{
    args::ValueReader reader;
    T val = 0;
    const bool success = reader.ParseNumericValue(input, val);
    if (success)
    {
        std::cerr << "FAIL: value with embedded NUL (size " << input.size()
                  << ") was accepted as " << static_cast<long long>(val)
                  << " but should have been rejected" << std::endl;
        exit(1);
    }
}

template <typename T>
static void expect_accept(const std::string &input, T expected)
{
    args::ValueReader reader;
    T val = 0;
    const bool success = reader.ParseNumericValue(input, val);
    if (!success)
    {
        std::cerr << "FAIL: valid value '" << input << "' was rejected" << std::endl;
        exit(1);
    }
    if (val != expected)
    {
        std::cerr << "FAIL: value '" << input << "' parsed as "
                  << static_cast<long long>(val) << " (expected "
                  << static_cast<long long>(expected) << ")" << std::endl;
        exit(1);
    }
}

int main()
{
    const std::string nul(1, '\0');

    // Data after an embedded NUL must not be silently dropped.
    expect_reject<int>("12" + nul + "garbage");
    expect_reject<long long>("34" + nul + "99");
    expect_reject<unsigned int>("12" + nul + "garbage");
    expect_reject<unsigned long long>("7" + nul + nul);
    // Trailing whitespace followed by an embedded NUL and junk: the
    // whitespace-tolerance loop must still stop at the true end.
    expect_reject<int>("12 " + nul + "x");
    // A NUL on its own is not a number.
    expect_reject<int>(nul);

    // Genuinely valid values, including the whitespace tolerance and the
    // base-0 hex/octal forms, must keep parsing.
    expect_accept<int>("12", 12);
    expect_accept<int>("  -7  ", -7);
    expect_accept<int>("0x10", 16);
    expect_accept<unsigned int>("010", 8);

    std::cout << "embedded NUL numeric tests passed!" << std::endl;
    return 0;
}
