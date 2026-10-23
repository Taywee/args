/*
 * Fuzzer for the shell completion protocol of args.hxx.
 *
 * The hosted parse_args_fuzzer never registers a CompletionFlag, so the
 * candidate-generation region of the parser was almost entirely unexercised
 * (its early exit was the only covered part, per the 2026-10-15 coverage
 * report).
 *
 * This harness registers a CompletionFlag alongside flags, value flags,
 * mapped flags/positionals, positionals, and commands, then issues valid
 * completion requests of the form
 *
 *     --completion <syntax> <cword> <prog> [words...]
 *
 * with mutated word indices (including malformed ones such as "1x", "-1",
 * "+1", " 2", "2 "), partial options ("", "-", "--", "-f", "--f", "-m1"),
 * separator/prefix overlaps ("--x", "---x", "--map="), mapped choices,
 * terminator ("--"), and command names. The long separator is also varied,
 * including values that overlap the "--" prefix, which historically drove
 * an out-of-range substr in the bash completion handler
 * (see test/completion_separator_prefix_overlap.cxx). Two requests are
 * issued per input to exercise re-parse between completions.
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

// Must be std::unordered_map: the Map* classes default their Map template
// parameter to args::detail::unordered_map and std::map does not convert.
const std::unordered_map<std::string, int> kChoices{{"alpha", 1}, {"beta", 2}};

const char *const kSeparators[] = {"=", "-", ":", "==", "=-"};

const char *const kCwords[] = {"1", "2", "3", "7", "0", "1x", "-1", "+1", " 2", "2 "};

const char *const kWords[] = {
    "", "-", "--", "-f", "--f", "--foo", "-b", "--bar", "--bar=", "-b=",
    "-m", "--map=", "-m1", "--x", "---x", "alpha", "beta", "--",
    "command1", "command2", "command3", "x",
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

    args::ArgumentParser parser("completion_fuzzer");
    args::CompletionFlag completion(parser, {"completion"});

    args::Flag foo(parser, "foo", "foo", {'f', "foo"});
    args::ValueFlag<std::string> bar(parser, "bar", "bar", {'b', "bar"}, "abc");
    args::ValueFlag<int> num(parser, "num", "num", {'n', "num"});
    args::MapFlag<std::string, int> map(parser, "map", "map", {'m', "map"}, kChoices);
    args::MapPositional<std::string, int> mpos(parser, "mpos", "mpos", kChoices);
    args::Positional<std::string> pos(parser, "pos", "pos");

    // Vary the long separator, including prefix-overlapping values.
    try
    {
        parser.LongSeparator(kSeparators[fdp.ConsumeIntegralInRange<size_t>(0, ArraySize(kSeparators) - 1)]);
    }
    catch (...)
    {
    }

    // Command subparsers, mirroring test/completion.cxx: arguments are
    // created inside the coroutine and Parse() is called before any result
    // is consumed; the objects stay alive for the duration of the parse.
    args::Command command1(parser, "command1", "first command", [](args::Subparser &sp)
    {
        args::ValueFlag<std::string> cfoo(sp, "cfoo", "cfoo", {'f', "cfoo"}, "abc");
        sp.Parse();
        if (cfoo)
        {
            (void)*cfoo;
        }
    });

    args::Command command2(parser, "command2", "second command", [](args::Subparser &sp)
    {
        sp.Parse();
    });

    for (int round = 0; round < 2; ++round)
    {
        std::vector<std::string> argv;
        argv.reserve(8);
        argv.emplace_back("--completion");
        if (fdp.ConsumeBool())
        {
            argv.emplace_back("bash");
        }
        else
        {
            argv.emplace_back(fdp.ConsumeRandomLengthString(8));
        }
        if (fdp.ConsumeBool())
        {
            argv.emplace_back(kCwords[fdp.ConsumeIntegralInRange<size_t>(0, ArraySize(kCwords) - 1)]);
        }
        else
        {
            argv.emplace_back(fdp.ConsumeRandomLengthString(4));
        }
        argv.emplace_back("prog");

        const size_t wordCount = fdp.ConsumeIntegralInRange<size_t>(0, 5);
        for (size_t i = 0; i < wordCount; ++i)
        {
            if (fdp.ConsumeBool())
            {
                argv.emplace_back(kWords[fdp.ConsumeIntegralInRange<size_t>(0, ArraySize(kWords) - 1)]);
            }
            else
            {
                argv.emplace_back(fdp.ConsumeRandomLengthString(12));
            }
        }

        try
        {
            parser.ParseArgs(argv);
        }
        catch (const args::Completion &)
        {
            // Normal completion control flow carrying the candidates.
        }
        catch (...)
        {
            // Malformed cword and other expected rejections.
        }
    }

    return 0;
}
