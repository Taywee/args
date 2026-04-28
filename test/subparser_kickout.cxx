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

    const std::vector<std::string> args{"--foo", "green", "--bar"};

    args::ArgumentParser parser1("Test command");
    args::Flag foo1(parser1, "Foo", "Foo", {'f', "foo"});
    args::Flag bar1(parser1, "Bar", "Bar", {'b', "bar"});
    args::MapPositional<std::string, MappingEnum> sub(parser1, "sub", "sub", map);
    sub.KickOut(true);

    auto next = parser1.ParseArgs(args);

    args::ArgumentParser parser2("Test command");
    args::Flag foo2(parser2, "Foo", "Foo", {'f', "foo"});
    args::Flag bar2(parser2, "Bar", "Bar", {'b', "bar"});

    parser2.ParseArgs(next, std::end(args));

    test::require(foo1);
    test::require_false(bar1);
    test::require(sub);
    test::require(*sub == MappingEnum::green);
    test::require_false(foo2);
    test::require(bar2);
    return 0;
}
