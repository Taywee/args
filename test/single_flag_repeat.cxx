/* Copyright (c) Taylor Richberger <taylor@axfive.net>
 * This code is released under the license described in the LICENSE file
 */

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

    args::ArgumentParser parser("Test command");
    args::Flag foo(parser, "Foo", "Foo", {'f', "foo"}, true);
    args::ValueFlag<std::string> bar(parser, "Bar", "Bar", {'b', "bar"}, "", true);
    args::Flag bix(parser, "Bix", "Bix", {'x', "bix"});
    args::MapFlag<std::string, MappingEnum> baz(parser, "Baz", "Baz", {'B', "baz"}, map, MappingEnum::def, true);
    test::require_throws_as<args::ExtraError>([&] { parser.ParseArgs(std::vector<std::string>{"--foo", "-f", "-bblah"}); });
    test::require_nothrow([&] { parser.ParseArgs(std::vector<std::string>{"--foo", "-xxx", "--bix", "-bblah", "--bix"}); });
    test::require_throws_as<args::ExtraError>([&] { parser.ParseArgs(std::vector<std::string>{"--foo", "-bblah", "-blah"}); });
    test::require_throws_as<args::ExtraError>([&] { parser.ParseArgs(std::vector<std::string>{"--foo", "-bblah", "--bar", "blah"}); });
    test::require_throws_as<args::ExtraError>([&] { parser.ParseArgs(std::vector<std::string>{"--baz=red", "-B", "yellow"}); });
    test::require_throws_as<args::ExtraError>([&] { parser.ParseArgs(std::vector<std::string>{"--baz", "red", "-Byellow"}); });
    test::require_nothrow([&] { parser.ParseArgs(std::vector<std::string>{"--foo", "-Bgreen"}); });
    test::require(foo);
    test::require_false(bar);
    test::require_false(bix);
    test::require(baz);
    test::require(*baz == MappingEnum::green);
    return 0;
}
