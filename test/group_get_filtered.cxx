/* Copyright (c) Taylor Richberger <taylor@axfive.net>
 * This code is released under the license described in the LICENSE file
 */

#include <args.hxx>

#include "test_helpers.hxx"

using StrPositional = args::Positional<std::string>;

int main()
{
    args::ArgumentParser parser("This is a test program.", "This goes after the options.");
    args::Flag flag_a(parser, "a", "test flag", {'a'});
    args::Flag flag_b(parser, "b", "test flag", {'b'});
    StrPositional position_1(parser, "first", "test positional");
    StrPositional position_2(parser, "second", "test positional");
    
    parser.ParseArgs(std::vector<std::string>{"-a", "first"});
    
    auto allFlags = parser.GetFilteredChildren<args::Flag>();
    test::require(allFlags.size() == 2);
    test::require_contains(allFlags, &flag_a);
    test::require_contains(allFlags, &flag_b);

    auto matchedFlags = parser.GetFilteredChildren<args::Flag>(true);
    test::require(matchedFlags.size() == 1);
    test::require(matchedFlags[0] == &flag_a);

    auto allPositional = parser.GetFilteredChildren<StrPositional>();
    test::require(allPositional.size() == 2);
    test::require_contains(allPositional, &position_1);
    test::require_contains(allPositional, &position_2);

    auto matchedPositional = parser.GetFilteredChildren<StrPositional>(true);
    test::require(matchedPositional.size() == 1);
    test::require(matchedPositional[0] == &position_1);
}