/* Copyright (c) Taylor Richberger <taylor@axfive.net>
 * This code is released under the license described in the LICENSE file
 */

#include "test_common.hxx"

#include <args.hxx>

#include "test_helpers.hxx"

int main()
{
    // With an empty long prefix the long form carries no prefix, but a short
    // flag must still be reachable through the (non-empty) short prefix rather
    // than being swallowed as a nameless long flag.
    {
        args::ArgumentParser parser("This is a test program.");
        parser.ShortPrefix("-");
        parser.LongPrefix("");
        parser.LongSeparator("=");
        args::ValueFlag<int> counter(parser, "integer", "The counter value", {'c', "counter"});
        args::Flag verbose(parser, "verbose", "Verbose output", {'v', "verbose"});
        parser.ParseArgs(std::vector<std::string>{"-v", "-c", "3"});
        test::require(verbose);
        test::require(counter);
        test::require(*counter == 3);
    }

    // The prefix-less long form keeps working unchanged.
    {
        args::ArgumentParser parser("This is a test program.");
        parser.ShortPrefix("-");
        parser.LongPrefix("");
        parser.LongSeparator("=");
        args::ValueFlag<int> counter(parser, "integer", "The counter value", {'c', "counter"});
        parser.ParseArgs(std::vector<std::string>{"counter=7"});
        test::require(counter);
        test::require(*counter == 7);
    }

    // The default "--"/"-" configuration is unaffected: a chunk starting with
    // both prefixes still resolves to the longer (long) one.
    {
        args::ArgumentParser parser("This is a test program.");
        args::Flag foo(parser, "foo", "The foo flag", {"foo"});
        args::Flag f(parser, "f", "The f flag", {'f'});
        parser.ParseArgs(std::vector<std::string>{"--foo", "-f"});
        test::require(foo);
        test::require(f);
    }

    return 0;
}
