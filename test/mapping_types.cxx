/* Copyright (c) Taylor Richberger <taylor@axfive.net>
 * This code is released under the license described in the LICENSE file
 */

#include "test_common.hxx"

#include <args.hxx>
#include <list>

#include "test_helpers.hxx"

struct CopyableValue
{
    int value;
    explicit CopyableValue(int value_ = 0) : value(value_) {}
    CopyableValue(const CopyableValue &) = default;
    CopyableValue(CopyableValue &&) = default;
    CopyableValue &operator=(const CopyableValue &) = default;
    CopyableValue &operator=(CopyableValue &&) = delete;
};

void CheckMappingListCopyableValue()
{
    const std::unordered_map<std::string, CopyableValue> map{{"one", CopyableValue(1)}};
    args::ArgumentParser parser("Copyable mapped values");
    args::MapFlagList<std::string, CopyableValue> flags(parser, "value", "Mapped flags", {'n'}, map);
    args::MapPositionalList<std::string, CopyableValue> positionals(parser, "value", "Mapped positionals", map);
    parser.ParseArgs(std::vector<std::string>{"-n", "one", "one"});
    test::require(flags->size() == 1);
    test::require(flags->front().value == 1);
    test::require(positionals->size() == 1);
    test::require(positionals->front().value == 1);
}

template <template <typename...> class List, template <typename...> class Map>
void CheckMappingListContainer()
{
    const Map<std::string, int> map{{"one", 1}, {"two", 2}};
    const List<int> defaults{9};
    const List<int> expected{2, 1, 2};
    args::ArgumentParser parser("Mapping list containers");
    args::MapFlagList<std::string, int, List, args::ValueReader, Map> flags(
        parser, "number", "Mapped flags", {'n', "number"}, map, defaults);
    args::MapPositionalList<std::string, int, List, args::ValueReader, Map> positionals(
        parser, "number", "Mapped positionals", map, defaults);

    parser.ParseArgs(std::vector<std::string>{});
    test::require_false(flags);
    test::require_false(positionals);
    test::require(*flags == defaults);
    test::require(*positionals == defaults);

    parser.ParseArgs(std::vector<std::string>{"-ntwo", "--number=one", "-n", "two", "two", "one", "two"});
    test::require(flags);
    test::require(positionals);
    test::require(*flags == expected);
    test::require(*positionals == expected);

    parser.Reset();
    test::require_false(flags);
    test::require_false(positionals);
    test::require(*flags == defaults);
    test::require(*positionals == defaults);

    test::require_throws_as<args::MapError>([&]
    {
        parser.ParseArgs(std::vector<std::string>{"-n", "one", "-n", "unknown"});
    });
    test::require(*flags == List<int>{1});
    test::require(*positionals == defaults);

    test::require_throws_as<args::MapError>([&]
    {
        parser.ParseArgs(std::vector<std::string>{"two", "unknown"});
    });
    test::require(*flags == defaults);
    test::require(*positionals == List<int>{2});
}

int main()
{
    CheckMappingListCopyableValue();
    CheckMappingListContainer<std::set, std::map>();
    CheckMappingListContainer<std::unordered_set, std::unordered_map>();
    CheckMappingListContainer<std::vector, std::unordered_map>();
    CheckMappingListContainer<std::list, std::map>();

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
