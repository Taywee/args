/* Copyright (c) Taylor Richberger <taylor@axfive.net>
 * This code is released under the license described in the LICENSE file
 */

#include "test_common.hxx"

#include <args.hxx>

#include "test_helpers.hxx"

int main()
{
    args::ArgumentParser parser("This is a test program.", "This goes after the options.");
    args::ValueFlag<std::string> foo(parser, "FOO", "test flag", {'f', "foo"});
    parser.ParseArgs(std::vector<std::string>{"--foo=test"});
    test::require(foo);
    test::require(*foo == "test");
    *foo = "bar";
    test::require(*foo == "bar");
    return 0;
}
