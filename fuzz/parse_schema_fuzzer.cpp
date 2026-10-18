/*
 * Combined schema-parsing harness for args.hxx (parse_schema_fuzzer).
 *
 * Supersedes the two upstream harnesses that were never built by OSS-Fuzz
 * because their names do not match the *_fuzzer.cpp discovery glob:
 *
 *   - fuzz_parser.cpp: called a nonexistent ParseArgs overload (so it
 *     cannot even compile) and lost empty/whitespace-only arguments to
 *     istringstream tokenization. Its intent - parse a full schema of
 *     untrusted arguments - lives on here.
 *   - fuzz_numeric_parser.cxx: renamed to numeric_parser_fuzzer.cpp and
 *     repaired as its own target; numeric conversion stays separate.
 *
 * Schema coverage: value flags over numeric and string types, implicit
 * values, bounded Nargs, repeated lists, mapped flags and positionals,
 * plain positionals, single-use and required options, the -- terminator,
 * defaults, and re-parsing. Tokens mix fuzzer-arbitrary strings with a
 * table of schema-recognized options so unknown-option rejection does not
 * dominate the parse.
 *
 * Crash oracle: ParseArgs legitimately throws only args::Error subtypes
 * (Help, Completion, ParseError, MapError, ValidationError, ExtraError).
 * Any other exception escaping the parser is a library bug - e.g. the
 * leaked std::out_of_range fixed by
 * test/completion_separator_prefix_overlap.cxx - and trips
 * __builtin_trap() below.
 */

#include <cstddef>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

#include <fuzzer/FuzzedDataProvider.h>

#include <args.hxx>

namespace
{

// NOTE: the Map* classes default their Map template parameter to
// args::detail::unordered_map (== std::unordered_map), so the choices map
// must be an unordered_map (a std::map does not convert).
const std::unordered_map<std::string, int> kModes{{"fast", 1}, {"slow", 2}, {"verbose", 3}};

const char *const kTokens[] = {
    "-h", "--help",
    "-f", "--flag",
    "-o", "-o",
    "-c", "-ccc",
    "-n", "7", "--number=42", "--number=-3", "--number=abc", "--number=",
    "-n0x10", "-n2147483648",
    "-r2", "-r", "0.5", "--ratio=1e3", "--ratio", "-r=2", "--ratio=inf",
    "-l", "abc", "--label=x", "--label=", "-l", "-f",
    "-j", "-j4", "-j", "4",
    "-N", "1", "--nums=2", "-N2", "--nums=x",
    "-P", "a", "-P", "-P", "b", "c",
    "-m", "fast", "--mode=slow", "--mode=unknown",
    "--modes=fast", "--modes", "verbose",
    "--req", "5", "--req",
    "src.txt", "alpha", "1", "2",
    "--", "-z", "--unknown", "", " ", "3.14", "-",
};

template <typename T, size_t N>
constexpr size_t ArraySize(T (&)[N])
{
    return N;
}

} // namespace

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    FuzzedDataProvider fdp(data, size);

    args::ArgumentParser parser("parse_schema_fuzzer", "typed value schema test");

    args::HelpFlag help(parser, "help", "help", {'h', "help"});
    args::Flag flag(parser, "flag", "flag", {'f', "flag"});
    args::Flag once(parser, "once", "once", {'o'}, args::Options::Single);
    args::CounterFlag count(parser, "count", "count", {'c', "count"});
    args::ValueFlag<int> number(parser, "number", "number", {'n', "number"}, 0);
    args::ValueFlag<double> ratio(parser, "ratio", "ratio", {"ratio"}, 0.0);
    args::ValueFlag<std::string> label(parser, "label", "label", {'l', "label"}, std::string("x"));
    args::ImplicitValueFlag<int> jobs(parser, "jobs", "jobs", {'j', "jobs"}, 0, 1);
    args::ValueFlagList<int> numbers(parser, "nums", "nums", {'N', "nums"});
    args::NargsValueFlag<std::string> pair(parser, "pair", "pair", {'P', "pair"}, args::Nargs(0, 2));
    args::MapFlag<std::string, int> mode(parser, "mode", "mode", {'m', "mode"}, kModes);
    args::MapFlagList<std::string, int> modes(parser, "modes", "modes", {"modes"}, kModes);
    args::ValueFlag<std::string> required(parser, "req", "req", {"req"}, args::Options::Required);
    args::Positional<std::string> src(parser, "src", "source");
    args::MapPositional<std::string, int> posMode(parser, "posmode", "mapped positional", kModes);
    args::PositionalList<std::string> dst(parser, "dst", "destinations");

    const size_t tokenCount = fdp.ConsumeIntegralInRange<size_t>(1, 14);
    std::vector<std::string> argv;
    argv.reserve(tokenCount);
    for (size_t i = 0; i < tokenCount; ++i)
    {
        if (fdp.ConsumeBool())
        {
            argv.emplace_back(kTokens[fdp.ConsumeIntegralInRange<size_t>(0, ArraySize(kTokens) - 1)]);
        }
        else
        {
            argv.emplace_back(fdp.ConsumeRandomLengthString(16));
        }
    }

    // ParseArgs legitimately throws only args::Error subtypes; anything
    // else escaping is a library bug and traps the fuzzer.
    try
    {
        parser.ParseArgs(argv);
    }
    catch (const args::Error &)
    {
    }
    catch (...)
    {
        __builtin_trap();
    }

    // -- Read back every typed result so getters and iterators instantiate ----
    (void)count.Get();
    if (number)
    {
        (void)*number;
    }
    if (ratio)
    {
        (void)*ratio;
    }
    if (label)
    {
        (void)*label;
    }
    if (jobs)
    {
        (void)*jobs;
    }
    if (numbers)
    {
        for (const int &value : numbers)
        {
            (void)value;
        }
    }
    if (pair)
    {
        for (const std::string &value : pair)
        {
            (void)value;
        }
    }
    if (mode)
    {
        (void)*mode;
    }
    if (modes)
    {
        for (const int &value : modes)
        {
            (void)value;
        }
    }
    if (src)
    {
        (void)*src;
    }
    if (posMode)
    {
        (void)*posMode;
    }
    if (dst)
    {
        for (const std::string &value : dst)
        {
            (void)value;
        }
    }

    // -- Re-parse pair: invalid parse (validation error) then known-good parse
    try
    {
        parser.ParseArgs(std::vector<std::string>{});
    }
    catch (const args::Error &)
    {
    }
    catch (...)
    {
        __builtin_trap();
    }

    try
    {
        parser.ParseArgs(std::vector<std::string>{"--req", "ok", "src.txt"});
    }
    catch (const args::Error &)
    {
    }
    catch (...)
    {
        __builtin_trap();
    }

    return 0;
}
