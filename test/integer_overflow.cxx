/* Copyright (c) Taylor Richberger <taylor@axfive.net>
 * This code is released under the license described in the LICENSE file
 */

#include "test_common.hxx"

#include <args.hxx>

#include "test_helpers.hxx"

int main()
{
    args::ArgumentParser parser("This is a test program.", "This goes after the options.");
    args::ValueFlag<int> foo(parser, "FOO", "test flag", {'f', "foo"});
    args::ValueFlag<unsigned int> uid(parser, "UID", "numeric id", {'u', "uid"});

    test::require_throws_as<args::ParseError>([&] { parser.ParseArgs(std::vector<std::string>{"--foo", "2147483648"}); });
    test::require_throws_as<args::ParseError>([&] { parser.ParseArgs(std::vector<std::string>{"--uid", "4294967296"}); });
    return 0;
}
