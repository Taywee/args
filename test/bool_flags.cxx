/* Copyright (c) Taylor Richberger <taylor@axfive.net>
 * This code is released under the license described in the LICENSE file
 */

#include "test_common.hxx"

#include <args.hxx>

#include "test_helpers.hxx"

int main()
{
    args::ArgumentParser parser("This is a test program.", "This goes after the options.");
    args::Flag foo(parser, "FOO", "test flag", {'f', "foo"});
    args::Flag bar(parser, "BAR", "test flag", {'b', "bar"});
    args::Flag baz(parser, "BAZ", "test flag", {'a', "baz"});
    args::Flag bix(parser, "BAZ", "test flag", {'x', "bix"});
    parser.ParseArgs(std::vector<std::string>{"--baz", "-fb"});
    test::require(foo);
    test::require(bar);
    test::require(baz);
    test::require_false(bix);
    return 0;
}
