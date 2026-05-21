/**
 * libFuzzer integration for args.hxx
 * 
 * Build with: cmake -DBUILD_FUZZERS=ON ..
 * Run with: ./fuzz_parser corpus/
 * 
 * This fuzzer tests argument parsing with malformed/untrusted input
 * to detect crashes, overflows, and undefined behavior.
 */

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>
#include <sstream>

// Include the args library
#include "../args.hxx"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    // Create a string from fuzzer input
    std::string input(reinterpret_cast<const char*>(data), size);
    
    // Simulate command-line arguments
    std::vector<std::string> args;
    
    // Parse input as space-separated arguments (simulate argv)
    std::istringstream iss(input);
    std::string arg;
    while (iss >> arg) {
        args.push_back(arg);
    }
    
    if (args.empty()) {
        return 0;  // Nothing to parse
    }
    
    // Build argv-style array
    std::vector<char*> argv;
    for (auto& a : args) {
        argv.push_back(const_cast<char*>(a.c_str()));
    }
    
    // Create argument parser
    args::ArgumentParser parser("Fuzzer test", "Fuzzing argument parsing");
    
    // Add various flag types to test
    args::HelpFlag help(parser, "help", "Display help", {'h', "help"});
    args::Flag verbose(parser, "verbose", "Enable verbose", {'v', "verbose"});
    args::ValueFlag<std::string> name(parser, "name", "User name", {'n', "name"});
    args::ValueFlag<int> count(parser, "count", "Count value", {'c', "count"});
    args::Positional<std::string> positional(parser, "input", "Input file");
    
    try {
        // Parse the arguments - this is what we're fuzzing
        parser.ParseArgs(static_cast<int>(argv.size()), argv.data());
    } catch (const args::Help&) {
        // Help requested - normal behavior
        return 0;
    } catch (const args::ParseError& e) {
        // Parse error - expected for malformed input
        return 0;
    } catch (const args::ValidationError& e) {
        // Validation error - also expected
        return 0;
    } catch (const std::exception& e) {
        // Any other exception is a potential bug
        __builtin_trap();  // Signal fuzzer this is a crash
    }
    
    return 0;  // Success
}