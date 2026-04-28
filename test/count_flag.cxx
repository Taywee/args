/* Copyright (c) Taylor Richberger <taylor@axfive.net>
 * This code is released under the license described in the LICENSE file
 */

#include "test_common.hxx"

#include <args.hxx>

#include "test_helpers.hxx"

int main()
{
    args::ArgumentParser parser("This is a test program.", "This goes after the options.");
    args::CounterFlag foo(parser, "FOO", "test flag", {'f', "foo"});
    args::CounterFlag bar(parser, "BAR", "test flag", {'b', "bar"}, 7);
    args::CounterFlag baz(parser, "BAZ", "test flag", {'z', "baz"}, 7);
    parser.ParseArgs(std::vector<std::string>{"--foo", "-fb", "--bar", "-b", "-f", "--foo"});
    test::require(foo);
    test::require(bar);
    test::require_false(baz);
    test::require(*foo == 4);
    test::require(*bar == 10);
    test::require(*baz == 7);
    return 0;
}
