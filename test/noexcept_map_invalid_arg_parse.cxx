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
        args::ArgumentParser parser("This is a test program.", "This goes after the options.");
        args::MapFlag<int, int> map(parser, "MAP", "map", {'m', "map"}, {{1, 1}, {2, 2}});
        parser.ParseArgs(std::vector<std::string>{"--map", "abc"});
        test::require(parser.GetError() == args::Error::Parse);
        (void)map;
    }

    {
        args::ArgumentParser parser("This is a test program.", "This goes after the options.");
        args::MapFlagList<int, int> maplist(parser, "MAPLIST", "maplist", {'l', "maplist"}, {{1, 1}, {2, 2}});
        parser.ParseArgs(std::vector<std::string>{"--maplist", "abc"});
        test::require(parser.GetError() == args::Error::Parse);
        (void)maplist;
    }

    {
        args::ArgumentParser parser("This is a test program.", "This goes after the options.");
        args::MapPositional<int, int, args::ValueReader, std::map> mappos(parser, "MAPPOS", "mappos", {{1, 1}, {2, 2}});
        parser.ParseArgs(std::vector<std::string>{"abc"});
        test::require(parser.GetError() == args::Error::Parse);
        (void)mappos;
    }

    {
        args::ArgumentParser parser("This is a test program.", "This goes after the options.");
        args::MapPositionalList<int, int, std::vector, args::ValueReader, std::map> mapposlist(parser, "MAPPOSLIST", "mapposlist", {{1, 1}, {2, 2}});
        parser.ParseArgs(std::vector<std::string>{"abc"});
        test::require(parser.GetError() == args::Error::Parse);
        (void)mapposlist;
    }
    return 0;
}