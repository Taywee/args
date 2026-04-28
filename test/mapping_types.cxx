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
    args::ArgumentParser parser("This is a test program.", "This goes after the options.");
    args::MapFlag<std::string, MappingEnum> dmf(parser, "DMF", "Maps string to an enum", {"dmf"}, map);
    args::MapFlag<std::string, MappingEnum> mf(parser, "MF", "Maps string to an enum", {"mf"}, map);
    args::MapFlag<std::string, MappingEnum, ToLowerReader> cimf(parser, "CIMF", "Maps string to an enum case-insensitively", {"cimf"}, map);
    args::MapFlagList<std::string, MappingEnum> mfl(parser, "MFL", "Maps string to an enum list", {"mfl"}, map);
    args::MapPositional<std::string, MappingEnum> mp(parser, "MP", "Maps string to an enum", map);
    args::MapPositionalList<std::string, MappingEnum> mpl(parser, "MPL", "Maps string to an enum list", map);
    parser.ParseArgs(std::vector<std::string>{"--mf=red", "--cimf=YeLLoW", "--mfl=bar", "foo", "--mfl=green", "red", "--mfl", "bar", "default"});
    test::require_false(dmf);
    test::require(*dmf == MappingEnum::def);
    test::require(mf);
    test::require(*mf == MappingEnum::red);
    test::require(cimf);
    test::require(*cimf == MappingEnum::yellow);
    test::require(mfl);
    test::require((*mfl == std::vector<MappingEnum>{MappingEnum::bar, MappingEnum::green, MappingEnum::bar}));
    test::require(mp);
    test::require((*mp == MappingEnum::foo));
    test::require(mpl);
    test::require((*mpl == std::vector<MappingEnum>{MappingEnum::red, MappingEnum::def}));
    test::require_throws_as<args::MapError>([&] { parser.ParseArgs(std::vector<std::string>{"--mf=YeLLoW"}); });
    return 0;
}
