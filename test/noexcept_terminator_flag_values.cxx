/* Copyright (c) Taylor Richberger <taylor@axfive.net>
 * This code is released under the license described in the LICENSE file
 */

#define ARGS_NOEXCEPT
#include "test_common.hxx"

#include <args.hxx>

#include "test_helpers.hxx"

int main()
{
    // Same defect under ARGS_NOEXCEPT: the "--" terminator was swallowed as a
    // value while the flag was still below its minimum, so parsing wrongly
    // succeeded with n = ["a", "--", "b", "c"]. It must instead stop at "--"
    // and report the below-minimum parse error.
    {
        args::ArgumentParser parser("Test command");
        args::NargsValueFlag<std::string> n(parser, "n", "", {'n'}, {2, 4});
        args::PositionalList<std::string> pos(parser, "pos", "");
        parser.ParseArgs(std::vector<std::string>{"-n", "a", "--", "b", "c"});
        test::require(parser.GetError() == args::Error::Parse);
        test::require(n->empty());
    }

    // Minimum satisfied before the terminator: parsing succeeds and the
    // trailing arguments become positionals.
    {
        args::ArgumentParser parser("Test command");
        args::NargsValueFlag<std::string> n(parser, "n", "", {'n'}, {1, 4});
        args::PositionalList<std::string> pos(parser, "pos", "");
        parser.ParseArgs(std::vector<std::string>{"-n", "a", "b", "--", "c", "d"});
        test::require(parser.GetError() == args::Error::None);
        test::require((*n == std::vector<std::string>{"a", "b"}));
        test::require((*pos == std::vector<std::string>{"c", "d"}));
    }

    return 0;
}
