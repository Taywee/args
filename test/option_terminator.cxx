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
    args::PositionalList<std::string> baz(parser, "BAZ", "test flag");
    args::Flag ofoo(parser, "FOO", "test flag", {'f', "foo"});
    args::Flag obar(parser, "BAR", "test flag", {"bar", 'b'});
    args::ValueFlag<double> obaz(parser, "BAZ", "test flag", {'a', "baz"});
    parser.ParseArgs(std::vector<std::string>{"--foo", "this is a test flag", "0", "a", "b", "--baz", "7.0", "c", "x", "y", "z"});
    test::require(foo);
    test::require((*foo == "this is a test flag"));
    test::require(bar);
    test::require(!*bar);
    test::require(baz);
    test::require((*baz == std::vector<std::string>{"a", "b", "c", "x", "y", "z"}));
    test::require(ofoo);
    test::require(!obar);
    test::require(obaz);
    parser.ParseArgs(std::vector<std::string>{"--foo", "this is a test flag", "0", "a", "--", "b", "--baz", "7.0", "c", "x", "y", "z"});
    test::require(foo);
    test::require((*foo == "this is a test flag"));
    test::require(bar);
    test::require(!*bar);
    test::require(baz);
    test::require((*baz == std::vector<std::string>{"a", "b", "--baz", "7.0", "c", "x", "y", "z"}));
    test::require(ofoo);
    test::require(!obar);
    test::require(!obaz);
    parser.ParseArgs(std::vector<std::string>{"--foo", "--", "this is a test flag", "0", "a", "b", "--baz", "7.0", "c", "x", "y", "z"});
    test::require(foo);
    test::require((*foo == "this is a test flag"));
    test::require(bar);
    test::require(!*bar);
    test::require(baz);
    test::require((*baz == std::vector<std::string>{"a", "b", "--baz", "7.0", "c", "x", "y", "z"}));
    test::require(ofoo);
    test::require(!obar);
    test::require(!obaz);
    return 0;
}
