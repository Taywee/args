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
    args::Flag bar(parser, "BAR", "test flag", {'b', "bar"});
    args::ValueFlag<double> baz(parser, "BAZ", "test flag", {'a', "baz"});
    args::ValueFlag<char> bim(parser, "BAZ", "test flag", {'B', "bim"});
    args::Flag bix(parser, "BAZ", "test flag", {'x', "bix"});
    parser.ParseArgs(std::vector<std::string>{"-bftest", "--baz=7.555e2", "--bim", "c"});
    test::require(foo);
    test::require(*foo == "test");
    test::require(bar);
    test::require(baz);
    test::require((*baz > 755.49 && *baz < 755.51));
    test::require(bim);
    test::require(*bim == 'c');
    test::require_false(bix);
    return 0;
}
