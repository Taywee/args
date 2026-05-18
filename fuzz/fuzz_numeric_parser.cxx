#include <string>
#include <cstdint>
#include <cstring>
#include <sstream>
#include "../args.hxx"

// Expose the ValueReader class for fuzzing
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    if (size == 0) return 0;
    
    // Create a null-terminated string from fuzzer input
    std::string input(reinterpret_cast<const char*>(data), size);
    
    // Test numeric parsing with various types
    args::ValueReader reader;
    
    // Test signed integer types
    {
        int dest_int = 0;
        reader("fuzz_test", input, dest_int);  // Should not crash
        
        short dest_short = 0;
        reader("fuzz_test", input, dest_short);  // Should not crash
        
        long long dest_ll = 0;
        reader("fuzz_test", input, dest_ll);  // Should not crash
    }
    
    // Test unsigned integer types
    {
        unsigned int dest_uint = 0;
        reader("fuzz_test", input, dest_uint);  // Should not crash
        
        unsigned short dest_ushort = 0;
        reader("fuzz_test", input, dest_ushort);  // Should not crash
        
        unsigned long long dest_ull = 0;
        reader("fuzz_test", input, dest_ull);  // Should not crash
    }
    
    // Test floating point types
    {
        float dest_float = 0.0f;
        reader("fuzz_test", input, dest_float);  // Should not crash
        
        double dest_double = 0.0;
        reader("fuzz_test", input, dest_double);  // Should not crash
    }
    
    return 0;
}