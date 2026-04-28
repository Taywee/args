/* Copyright (c) Taylor Richberger <taylor@axfive.net>
 * This code is released under the license described in the LICENSE file
 */

#include "test_common.hxx"

#include <args.hxx>

#include "test_helpers.hxx"

int main()
{
    args::ArgumentParser parser("This is a test program.", "This goes after the options.");
    args::ValueFlagList<std::string, std::unordered_set> foo(parser, "FOO", "test flag", {'f', "foo"});
    parser.ParseArgs(std::vector<std::string>{"--foo=7", "-fblah", "-f", "9", "--foo", "blah"});
    test::require((*foo == std::unordered_set<std::string>{"7", "9", "blah"}));
    return 0;
}
