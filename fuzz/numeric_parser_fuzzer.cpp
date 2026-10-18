#include <string>
#include <cstdint>
#include <args.hxx>

/*
 * Numeric conversion harness for args::ValueReader.
 *
 * Repaired for OSS-Fuzz: ValueReader::operator() throws args::ParseError
 * for any input the requested type cannot represent, and the original
 * harness had no exception handling, so the first malformed byte string
 * aborted the whole target before later conversions could run.
 *
 * Each conversion is now attempted independently, with the expected
 * rejection swallowed per type, so a single input reaches every numeric
 * path: strtoll/strtoull-based integers (with range and narrowing
 * checks), sign rejection for unsigned types, trailing-whitespace
 * tolerance, and istringstream-based parsing for bool, char, and
 * floating-point types. Embedded NUL bytes are preserved on purpose:
 * the readers deliberately distinguish "12\0junk" from "12".
 */

// Accumulates converted values so the conversions cannot be optimized away.
unsigned long g_sink = 0;

template <typename T>
static T AttemptConversion(args::ValueReader &reader, const std::string &input)
{
    T destination{};
    try
    {
        reader("fuzz_test", input, destination);
    }
    catch (...)
    {
        // Expected rejection for values this type cannot represent.
    }
    return destination;
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    // Create a string from fuzzer input, preserving embedded NULs
    std::string input(reinterpret_cast<const char*>(data), size);

    args::ValueReader reader;

    // Character-ish types (istringstream-based paths)
    g_sink += static_cast<unsigned long>(AttemptConversion<bool>(reader, input));
    g_sink += static_cast<unsigned long>(AttemptConversion<char>(reader, input));
    g_sink += static_cast<unsigned long>(AttemptConversion<signed char>(reader, input));
    g_sink += static_cast<unsigned long>(AttemptConversion<unsigned char>(reader, input));

    // Test signed integer types (strtoll-based path, narrowing checks)
    g_sink += static_cast<unsigned long>(AttemptConversion<short>(reader, input));
    g_sink += static_cast<unsigned long>(AttemptConversion<int>(reader, input));
    g_sink += static_cast<unsigned long>(AttemptConversion<long>(reader, input));
    g_sink += static_cast<unsigned long>(AttemptConversion<long long>(reader, input));

    // Test unsigned integer types (strtoull-based path, sign rejection)
    g_sink += static_cast<unsigned long>(AttemptConversion<unsigned short>(reader, input));
    g_sink += static_cast<unsigned long>(AttemptConversion<unsigned int>(reader, input));
    g_sink += static_cast<unsigned long>(AttemptConversion<unsigned long>(reader, input));
    g_sink += static_cast<unsigned long>(AttemptConversion<unsigned long long>(reader, input));

    // Test floating point types (istringstream-based path)
    g_sink += static_cast<unsigned long>(AttemptConversion<float>(reader, input));
    g_sink += static_cast<unsigned long>(AttemptConversion<double>(reader, input));
    g_sink += static_cast<unsigned long>(AttemptConversion<long double>(reader, input));

    return 0;
}