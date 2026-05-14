#include "test_common.hxx"
#include <args.hxx>
#include "test_helpers.hxx"
#include <iostream>
#include <string>

template <typename T>
void test_parse(const std::string& input, bool expect_success, T expected_val) {
    args::ValueReader reader;
    T val = 0;
    bool success = reader.ParseNumericValue(input, val);
    
    if (success != expect_success) {
        std::cerr << "FAIL (success mismatch): input='" << input 
                  << "' success=" << success << " (expected " << expect_success << ")" << std::endl;
        exit(1);
    }
    
    if (success && val != expected_val) {
        std::cerr << "FAIL (value mismatch): input='" << input 
                  << "' val=" << (long long)val << " (expected " << (long long)expected_val << ")" << std::endl;
        exit(1);
    }
}

int main() {
    // Regression tests for C++17 from_chars compatibility
    // These cases were historically supported via strtoll(..., 0) but failed in naive C++17 std::from_chars
    
    test_parse<int>("+123", true, 123);   // Explicit plus sign
    test_parse<int>("0x10", true, 16);   // Hex auto-detection
    test_parse<int>("-0x10", true, -16); // Signed hex
    test_parse<int>("010", true, 8);     // Octal auto-detection
    test_parse<int>("08", false, 0);     // Invalid octal digits must be rejected
    
    std::cout << "All regression tests passed!" << std::endl;
    return 0;
}
