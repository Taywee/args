/* Copyright (c) Taylor Richberger <taylor@axfive.net>
 * This code is released under the license described in the LICENSE file
 */

#define ARGS_NOEXCEPT
#include "test_common.hxx"

#include <args.hxx>

#include "test_helpers.hxx"

int main()
{
    {
        args::ArgumentParser parser("Test command");
        args::NargsValueFlag<int> nums(parser, "NUMS", "test", {'n', "nums"}, {1, 2}, {42});

        parser.ParseArgs(std::vector<std::string>{"--nums", "abc"});
        test::require(parser.GetError() == args::Error::Parse);
        test::require((*nums == std::vector<int>{}));
    }

    {
        args::ArgumentParser parser("Test command");
        args::ValueFlagList<int> vals(parser, "VAL", "test", {'v', "vals"}, {11});

        parser.ParseArgs(std::vector<std::string>{"--vals", "abc"});
        test::require(parser.GetError() == args::Error::Parse);
        test::require((*vals == std::vector<int>{}));
    }

    {
        args::ArgumentParser parser("Test command");
        args::Positional<int> pos(parser, "POS", "test", 123);

        parser.ParseArgs(std::vector<std::string>{"abc"});
        test::require(parser.GetError() == args::Error::Parse);
        test::require(*pos == 123);
        test::require_false(static_cast<bool>(pos));
    }

    {
        args::ArgumentParser parser("Test command");
        args::Positional<int> first(parser, "FIRST", "test", 123);
        args::Positional<int> second(parser, "SECOND", "test", 456);

        parser.ParseArgs(std::vector<std::string>{"abc", "10"});
        test::require(parser.GetError() == args::Error::Parse);
        test::require(*first == 123);
        test::require_false(static_cast<bool>(first));
        test::require(*second == 456);
        test::require_false(static_cast<bool>(second));
    }

    {
        args::ArgumentParser parser("Test command");
        args::PositionalList<int> poslist(parser, "POSLIST", "test", {99});

        parser.ParseArgs(std::vector<std::string>{"abc", "10"});
        test::require(parser.GetError() == args::Error::Parse);
        test::require((*poslist == std::vector<int>{}));
        test::require_false(static_cast<bool>(poslist));
    }


    return 0;
}
