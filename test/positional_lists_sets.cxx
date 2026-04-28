/* Copyright (c) Taylor Richberger <taylor@axfive.net>
 * This code is released under the license described in the LICENSE file
 */

#include "test_common.hxx"

#include <args.hxx>

#include "test_helpers.hxx"

int main()
{
    args::ArgumentParser parser("This is a test program.", "This goes after the options.");
    args::PositionalList<std::string, std::unordered_set> foo(parser, "FOO", "test positional");
    parser.ParseArgs(std::vector<std::string>{"foo", "FoO", "bar", "baz", "foo", "9", "baz"});
    test::require((*foo == std::unordered_set<std::string>{"foo", "FoO", "bar", "baz", "9"}));
    return 0;
}
