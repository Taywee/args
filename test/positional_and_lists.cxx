/* Copyright (c) Taylor Richberger <taylor@axfive.net>
 * This code is released under the license described in the LICENSE file
 */

#include "test_common.hxx"

#include <args.hxx>

#include "test_helpers.hxx"

int main()
{
    args::ArgumentParser parser("This is a test program.", "This goes after the options.");
    args::Positional<std::string> foo(parser, "FOO", "test flag");
    args::Positional<bool> bar(parser, "BAR", "test flag");
    args::PositionalList<char> baz(parser, "BAZ", "test flag");
    parser.ParseArgs(std::vector<std::string>{"this is a test flag", "0", "a", "b", "c", "x", "y", "z"});
    test::require(foo);
    test::require((*foo == "this is a test flag"));
    test::require(bar);
    test::require(!*bar);
    test::require(baz);
    test::require((*baz == std::vector<char>{'a', 'b', 'c', 'x', 'y', 'z'}));
    return 0;
}
