/* Copyright (c) Taylor Richberger <taylor@axfive.net>
 * This code is released under the license described in the LICENSE file
 */

#include "test_common.hxx"

#include <args.hxx>

#include "test_helpers.hxx"

int main()
{
    // While a flag is still gathering values below its minimum, the "--"
    // terminator was consumed as a value instead of ending option parsing.
    // "-n a -- b c" collected ["a", "--", "b", "c"] and left the positional
    // list empty. "--" must stop the collection, so "n" only receives "a",
    // which is below the minimum of 2.
    {
        args::ArgumentParser parser("Test command");
        args::NargsValueFlag<std::string> n(parser, "n", "", {'n'}, {2, 4});
        args::PositionalList<std::string> pos(parser, "pos", "");
        test::require_throws_as<args::ParseError>([&] { parser.ParseArgs(std::vector<std::string>{"-n", "a", "--", "b", "c"}); });
    }

    // A single-value flag immediately followed by the terminator receives no
    // value rather than the literal "--".
    {
        args::ArgumentParser parser("Test command");
        args::ValueFlag<std::string> n(parser, "n", "", {'n'});
        args::PositionalList<std::string> pos(parser, "pos", "");
        test::require_throws_as<args::ParseError>([&] { parser.ParseArgs(std::vector<std::string>{"-n", "--", "x"}); });
    }

    // Once the minimum is satisfied, "--" still terminates and the following
    // arguments become positionals.
    {
        args::ArgumentParser parser("Test command");
        args::NargsValueFlag<std::string> n(parser, "n", "", {'n'}, {1, 4});
        args::PositionalList<std::string> pos(parser, "pos", "");
        test::require_nothrow([&] { parser.ParseArgs(std::vector<std::string>{"-n", "a", "b", "--", "c", "d"}); });
        test::require((*n == std::vector<std::string>{"a", "b"}));
        test::require((*pos == std::vector<std::string>{"c", "d"}));
    }

    return 0;
}
