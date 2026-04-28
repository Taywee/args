/* Copyright (c) Taylor Richberger <taylor@axfive.net>
 * This code is released under the license described in the LICENSE file
 */

#define ARGS_NOEXCEPT
#include "test_common.hxx"

#include <args.hxx>

#include "test_helpers.hxx"

int main()
{
    std::unordered_map<std::string, MappingEnum> map{
        {"default", MappingEnum::def},
        {"foo", MappingEnum::foo},
        {"bar", MappingEnum::bar},
        {"red", MappingEnum::red},
        {"yellow", MappingEnum::yellow},
        {"green", MappingEnum::green}};

    args::ArgumentParser parser("This is a test program.", "This goes after the options.");
    args::HelpFlag help(parser, "help", "Display this help menu", {'h', "help"});
    args::Flag bar(parser, "BAR", "test flag", {'b', "bar"}, true);
    args::ValueFlag<int> foo(parser, "FOO", "test flag", {'f', "foo"});
    args::Group nandgroup(parser, "this group provides nand validation", args::Group::Validators::AtMostOne);
    args::Flag x(nandgroup, "x", "test flag", {'x'});
    args::Flag y(nandgroup, "y", "test flag", {'y'});
    args::Flag z(nandgroup, "z", "test flag", {'z'});
    args::MapFlag<std::string, MappingEnum> mf(parser, "MF", "Maps string to an enum", {"mf"}, map);
    parser.ParseArgs(std::vector<std::string>{"-h"});
    test::require(parser.GetError() == args::Error::Help);
    parser.ParseArgs(std::vector<std::string>{"--Help"});
    test::require(parser.GetError() == args::Error::Parse);
    parser.ParseArgs(std::vector<std::string>{"--bar=test"});
    test::require(parser.GetError() == args::Error::Parse);
    parser.ParseArgs(std::vector<std::string>{"--bar"});
    test::require(parser.GetError() == args::Error::None);
    parser.ParseArgs(std::vector<std::string>{"--bar", "-b"});
    test::require(parser.GetError() == args::Error::Extra);

    parser.ParseArgs(std::vector<std::string>{"--foo=7.5"});
    test::require(parser.GetError() == args::Error::Parse);
    parser.ParseArgs(std::vector<std::string>{"--foo", "7a"});
    test::require(parser.GetError() == args::Error::Parse);
    parser.ParseArgs(std::vector<std::string>{"--foo", "7e4"});
    test::require(parser.GetError() == args::Error::Parse);
    parser.ParseArgs(std::vector<std::string>{"--foo"});
    test::require(parser.GetError() == args::Error::Parse);

    parser.ParseArgs(std::vector<std::string>{"--foo=85"});
    test::require(parser.GetError() == args::Error::None);

    parser.ParseArgs(std::vector<std::string>{"this is a test flag again", "1", "this has no positional available"});
    test::require(parser.GetError() == args::Error::Parse);

    parser.ParseArgs(std::vector<std::string>{"-x"});
    test::require(parser.GetError() == args::Error::None);
    parser.ParseArgs(std::vector<std::string>{"-xz"});
    test::require(parser.GetError() == args::Error::Validation);
    parser.ParseArgs(std::vector<std::string>{"-y"});
    test::require(parser.GetError() == args::Error::None);
    parser.ParseArgs(std::vector<std::string>{"-y", "-xz"});
    test::require(parser.GetError() == args::Error::Validation);
    parser.ParseArgs(std::vector<std::string>{"--mf", "YeLLoW"});
    test::require(parser.GetError() == args::Error::Map);
    parser.ParseArgs(std::vector<std::string>{"--mf", "yellow"});
    test::require(parser.GetError() == args::Error::None);
    return 0;
}
