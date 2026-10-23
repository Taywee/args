/* Copyright (c) Taylor Richberger <taylor@axfive.net>
 * This code is released under the license described in the LICENSE file
 */

#include "test_common.hxx"

#include <args.hxx>

#include "test_helpers.hxx"

int main()
{
    // Separate values must still be refused when SetArgumentSeparations turns
    // them off, even for flags whose argument is optional (Nargs min == 0).
    {
        args::ArgumentParser parser("This is a test program.");
        args::ImplicitValueFlag<int> jobs(parser, "N", "jobs", {'j', "jobs"}, 8, 0);
        args::Positional<std::string> pos(parser, "POS", "positional");
        parser.SetArgumentSeparations(true, true, true, false);
        test::require_nothrow([&] { parser.ParseArgs(std::vector<std::string>{"--jobs", "3"}); });
        test::require(args::get(jobs) == 8);
        test::require(args::get(pos) == "3");
    }
    {
        args::ArgumentParser parser("This is a test program.");
        args::ImplicitValueFlag<int> jobs(parser, "N", "jobs", {'j', "jobs"}, 8, 0);
        args::Positional<std::string> pos(parser, "POS", "positional");
        parser.SetArgumentSeparations(true, true, false, true);
        test::require_nothrow([&] { parser.ParseArgs(std::vector<std::string>{"-j", "3"}); });
        test::require(args::get(jobs) == 8);
        test::require(args::get(pos) == "3");
    }

    // A joined value doesn't license the remaining arguments to be picked up
    // separately when separate values are disallowed.
    {
        args::ArgumentParser parser("This is a test program.");
        args::NargsValueFlag<std::string> nums(parser, "N", "nums", {'n', "nums"}, args::Nargs{2, 3});
        parser.SetArgumentSeparations(true, true, true, false);
        test::require_throws_as<args::ParseError>([&] { parser.ParseArgs(std::vector<std::string>{"--nums=1", "2"}); });
    }

    // With joined short values off, -nf leaves the chunk to be parsed as flags,
    // so n takes no argument and f is still matched.
    {
        args::ArgumentParser parser("This is a test program.");
        args::NargsValueFlag<std::string> nums(parser, "N", "nums", {'n', "nums"}, args::Nargs{0, 2});
        args::Flag f(parser, "f", "test flag", {'f'});
        args::Positional<std::string> pos(parser, "POS", "positional");
        parser.SetArgumentSeparations(false, true, true, true);
        test::require_nothrow([&] { parser.ParseArgs(std::vector<std::string>{"-nf", "5"}); });
        test::require(args::get(nums).empty());
        test::require(bool(f));
        test::require(args::get(pos) == "5");
    }

    // Nargs must be separated.
    {
        args::ArgumentParser parser("This is a test program.");
        args::NargsValueFlag<int> a(parser, "A", "a", {'a', "aa"}, 2);
        test::require_nothrow([&] { parser.ParseArgs(std::vector<std::string>{"-a", "1", "2"}); });
        test::require((args::get(a) == std::vector<int>{1, 2}));
        test::require_nothrow([&] { parser.ParseArgs(std::vector<std::string>{"--aa", "3", "4"}); });
        test::require((args::get(a) == std::vector<int>{3, 4}));
        test::require_throws_as<args::ParseError>([&] { parser.ParseArgs(std::vector<std::string>{"-a1", "2"}); });
        test::require_throws_as<args::ParseError>([&] { parser.ParseArgs(std::vector<std::string>{"--a=1", "2"}); });
    }
    return 0;
}
