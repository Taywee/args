/* Copyright (c) Taylor Richberger <taylor@axfive.net>
 * This code is released under the license described in the LICENSE file
 *
 * Regression test: when a flag carrying Options::Single is passed twice in
 * ARGS_NOEXCEPT mode, FlagBase::Match used to set Error::Extra and then fall
 * through to the normal value-parsing path, silently overwriting the
 * previously stored value (and, for CounterFlag, double-counting the
 * duplicate).  Non-noexcept mode throws ExtraError before the second value
 * is parsed, so the prior value is preserved.  The two modes must agree:
 * after a duplicate Single flag, GetError() reports Extra and the stored
 * value reflects only the first, accepted occurrence.
 */

#define ARGS_NOEXCEPT
#include "test_common.hxx"

#include <args.hxx>

#include "test_helpers.hxx"

int main()
{
    // ValueFlag<std::string> + Options::Single: second value must not overwrite.
    {
        args::ArgumentParser parser("Test command");
        args::ValueFlag<std::string> f(parser, "X", "test", {'x'}, "default", args::Options::Single);

        parser.ParseArgs(std::vector<std::string>{"-x", "first", "-x", "second"});
        test::require(parser.GetError() == args::Error::Extra);
        test::require(*f == "first");
    }

    // Long-form flag through ParseLong path.
    {
        args::ArgumentParser parser("Test command");
        args::ValueFlag<std::string> f(parser, "X", "test", {"xx"}, "default", args::Options::Single);

        parser.ParseArgs(std::vector<std::string>{"--xx", "first", "--xx=second"});
        test::require(parser.GetError() == args::Error::Extra);
        test::require(*f == "first");
    }

    // ValueFlag<int>: ensure numeric value-readers are not invoked the second time.
    {
        args::ArgumentParser parser("Test command");
        args::ValueFlag<int> f(parser, "N", "test", {'n'}, 0, args::Options::Single);

        parser.ParseArgs(std::vector<std::string>{"-n", "1", "-n", "2"});
        test::require(parser.GetError() == args::Error::Extra);
        test::require(*f == 1);
    }

    // CounterFlag: count must not advance past the duplicate.
    {
        args::ArgumentParser parser("Test command");
        args::CounterFlag c(parser, "C", "test", {'c'}, 0, args::Options::Single);

        parser.ParseArgs(std::vector<std::string>{"-c", "-c"});
        test::require(parser.GetError() == args::Error::Extra);
        test::require(*c == 1);
    }

    // MapFlag: the mapped value must reflect only the accepted first match.
    {
        args::ArgumentParser parser("Test command");
        std::unordered_map<std::string, int> m{{"one", 1}, {"two", 2}};
        args::MapFlag<std::string, int> f(parser, "M", "test", {'m'}, m, 0, args::Options::Single);

        parser.ParseArgs(std::vector<std::string>{"-m", "one", "-m", "two"});
        test::require(parser.GetError() == args::Error::Extra);
        test::require(*f == 1);
    }

    return 0;
}
