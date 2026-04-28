/* Copyright (c) Taylor Richberger <taylor@axfive.net>
 * This code is released under the license described in the LICENSE file
 */

#include "test_common.hxx"

#include <args.hxx>

#include "test_helpers.hxx"

int main()
{
    args::ArgumentParser parser("This is a test program.", "This goes after the options.");
    args::ValueFlagList<int> foo(parser, "FOO", "test flag", {'f', "foo"}, {9, 7, 5});
    parser.ParseArgs(std::vector<std::string>{"--foo=7", "-f2", "-f", "9", "--foo", "42"});
    test::require((*foo == std::vector<int>{7, 2, 9, 42}));
    return 0;
}
