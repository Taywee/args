/* Copyright (c) Taylor Richberger <taylor@axfive.net>
 * This code is released under the license described in the LICENSE file
 */

#include "test_common.hxx"

#include <args.hxx>

#include "test_helpers.hxx"

int main()
{
    args::ArgumentParser parser("This is a test program.", "This goes after the options.");
    args::PositionalList<int> foo(parser, "FOO", "test flag", {9, 7, 5});
    parser.ParseArgs(std::vector<std::string>{"7", "2", "9", "42"});
    test::require((*foo == std::vector<int>{7, 2, 9, 42}));
    return 0;
}
