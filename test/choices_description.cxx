/* Copyright (c) Taylor Richberger <taylor@axfive.net>
 * This code is released under the license described in the LICENSE file
 */

#include "test_common.hxx"

#include <args.hxx>

#include "test_helpers.hxx"

int main()
{
    args::ArgumentParser p("parser");
    args::MapFlag<int, int> map(p, "map", "map", {"map"}, {{1,1}, {2, 2}});
    args::MapFlagList<char, int> maplist(p, "maplist", "maplist", {"maplist"}, {{'1',1}, {'2', 2}});
    args::MapPositional<std::string, int, args::ValueReader, std::map> mappos(p, "mappos", "mappos", {{"1",1}, {"2", 2}});
    args::MapPositionalList<char, int, std::vector, args::ValueReader, std::map> mapposlist(p, "mapposlist", "mapposlist", {{'1',1}, {'2', 2}});

    test::require(map.HelpChoices(p.helpParams) == std::vector<std::string>{"1", "2"});
    test::require(maplist.HelpChoices(p.helpParams) == std::vector<std::string>{"1", "2"});
    test::require(mappos.HelpChoices(p.helpParams) == std::vector<std::string>{"1", "2"});
    test::require(mapposlist.HelpChoices(p.helpParams) == std::vector<std::string>{"1", "2"});
    return 0;
}
