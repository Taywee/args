/* Copyright (c) Taylor Richberger <taylor@axfive.net>
 * This code is released under the license described in the LICENSE file
 *
 * Regression test: a flag's per-flag error set by ParseValue (Help, Parse,
 * Map) must not be shadowed by a later parser-level error in ARGS_NOEXCEPT
 * mode. In non-noexcept mode the corresponding ParseValue throws and parsing
 * stops; the noexcept path must behave equivalently.
 */

#define ARGS_NOEXCEPT
#include "test_common.hxx"

#include <args.hxx>

#include "test_helpers.hxx"

int main()
{
    // HelpFlag followed by an unknown long flag: the user asked for help, so
    // GetError() must report Help, not Parse from the unknown flag.
    {
        args::ArgumentParser parser("Test");
        args::HelpFlag help(parser, "help", "show help", {"help"});
        parser.ParseArgs(std::vector<std::string>{"--help", "--unknown"});
        test::require(parser.GetError() == args::Error::Help);
        test::require(parser.GetErrorMsg() == "help");
    }

    // Same scenario via short flag.
    {
        args::ArgumentParser parser("Test");
        args::HelpFlag help(parser, "help", "show help", {'h'});
        parser.ParseArgs(std::vector<std::string>{"-h", "--unknown"});
        test::require(parser.GetError() == args::Error::Help);
        test::require(parser.GetErrorMsg() == "help");
    }

    // MapFlag with a key not in the map followed by an unknown flag: the
    // Map error originates from the bad key, not Parse from the unknown.
    {
        args::ArgumentParser parser("Test");
        args::MapFlag<std::string, int> m(parser, "M", "map flag", {'m', "map"}, {{"a", 1}});
        parser.ParseArgs(std::vector<std::string>{"--map=bad", "--unknown"});
        test::require(parser.GetError() == args::Error::Map);
    }

    // MapFlag short-form variant.
    {
        args::ArgumentParser parser("Test");
        args::MapFlag<std::string, int> m(parser, "M", "map flag", {'m'}, {{"a", 1}});
        parser.ParseArgs(std::vector<std::string>{"-mbad", "--unknown"});
        test::require(parser.GetError() == args::Error::Map);
    }

    // Reader failure followed by an unknown flag: the parser-level errorMsg
    // for the unknown flag must not displace the original value-parse error.
    {
        args::ArgumentParser parser("Test");
        args::ValueFlag<int> num(parser, "NUM", "number", {"num"});
        parser.ParseArgs(std::vector<std::string>{"--num=abc", "--unknown"});
        test::require(parser.GetError() == args::Error::Parse);
        // Before the fix, errorMsg would be "Flag could not be matched: unknown".
        test::require(parser.GetErrorMsg().find("unknown") == std::string::npos);
    }

    // Without any trailing arg, the flag's error is reported as before
    // (sanity check that the fix does not regress the simple path).
    {
        args::ArgumentParser parser("Test");
        args::HelpFlag help(parser, "help", "show help", {"help"});
        parser.ParseArgs(std::vector<std::string>{"--help"});
        test::require(parser.GetError() == args::Error::Help);
    }

    return 0;
}
