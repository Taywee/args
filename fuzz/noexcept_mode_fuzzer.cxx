/*
 * ARGS_NOEXCEPT compile-variant harness for args.hxx.
 *
 * Defining ARGS_NOEXCEPT before including args.hxx selects a different
 * compile-time configuration of the library: instead of throwing, the
 * parser records an error status that callers read back with GetError()
 * and GetErrorMsg(). This selects alternative code paths throughout the
 * whole library (parser, readers, completion, subparsers), so it is built
 * as a separate binary rather than fed through the same targets.
 *
 * The harness mirrors the contract of test/noexcept_mode.cxx and
 * test/noexcept_completion.cxx: help, parse, validation, map, and extra
 * errors, mapped flags, required options, a validated group, commands,
 * completion requests, and a failure-to-success re-parse to exercise the
 * error-status reset between parses.
 *
 * NOTE: this file deliberately defines ARGS_NOEXCEPT itself and uses the
 * error-status API, so it MUST NOT be compiled without the macro and is
 * intentionally named *.cxx to stay outside the *_fuzzer.cpp discovery
 * glob; the oss-fuzz build script compiles it explicitly.
 */

#define ARGS_NOEXCEPT

#include <cstddef>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

#include <fuzzer/FuzzedDataProvider.h>

#include <args.hxx>

namespace
{

enum class Mapping
{
    def,
    foo,
    bar
};

const std::unordered_map<std::string, Mapping> kMap{
    {"default", Mapping::def}, {"foo", Mapping::foo}, {"bar", Mapping::bar}};

const char *const kTokens[] = {
    "-h", "--Help", "--bar", "-b", "--bar=test", "--bar", "-b",
    "--foo=7.5", "--foo", "85", "7a", "7e4", "--foo=",
    "--mf", "yellow", "YeLLoW", "--mf=default",
    "-x", "-xz", "-y", "-y", "-x",
    "--req", "value", "cmd", "-n", "3",
    "--completion", "bash", "1", "prog", "-", "--map=",
    "--", "extra", "1", "", "-z", "garbage",
};

template <typename T, size_t N>
constexpr size_t ArraySize(T (&)[N])
{
    return N;
}

std::vector<std::string> BuildArgv(FuzzedDataProvider &fdp)
{
    const size_t tokenCount = fdp.ConsumeIntegralInRange<size_t>(0, 10);
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
    return argv;
}

} // namespace

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    FuzzedDataProvider fdp(data, size);

    args::ArgumentParser parser("noexcept_mode_fuzzer", "epilog");
    args::HelpFlag help(parser, "help", "help", {'h', "help"});
    args::Flag bar(parser, "bar", "bar", {'b', "bar"}, true);
    args::ValueFlag<int> foo(parser, "foo", "foo", {'f', "foo"});
    args::Group nand(parser, "at-most-one group", args::Group::Validators::AtMostOne);
    args::Flag x(nand, "x", "x", {'x'});
    args::Flag y(nand, "y", "y", {'y'});
    args::MapFlag<std::string, Mapping> mf(parser, "mf", "mf", {"mf"}, kMap);
    args::ValueFlag<std::string> required(parser, "req", "req", {"req"}, args::Options::Required);
    args::CompletionFlag completion(parser, {"completion"});

    args::Command cmd(parser, "cmd", "subcommand", [](args::Subparser &sp)
    {
        args::ValueFlag<int> num(sp, "num", "num", {'n', "num"});
        sp.Parse();
        if (num)
        {
            (void)*num;
        }
    });

    parser.RequireCommand(fdp.ConsumeBool());

    // First parse: arbitrary token mix; read back the error status.
    parser.ParseArgs(BuildArgv(fdp));
    const args::Error first = parser.GetError();
    (void)parser.GetErrorMsg();

    // Consume results in the non-throwing configuration.
    if (bar)
    {
        (void)bar.Get();
    }
    if (foo)
    {
        (void)*foo;
    }
    if (mf)
    {
        (void)*mf;
    }
    if (required)
    {
        (void)*required;
    }
    (void)args::get(completion);

    // Failure-to-success (and success-to-failure) re-parse: exercises the
    // error-status reset carried by ParseArgs between parses.
    if (first == args::Error::None)
    {
        parser.ParseArgs(std::vector<std::string>{"--foo=abc"});
    }
    else
    {
        parser.ParseArgs(std::vector<std::string>{"--req", "ok", "cmd"});
    }
    (void)parser.GetError();
    (void)parser.GetErrorMsg();

    return 0;
}
