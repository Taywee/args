/* Copyright (c) Taylor Richberger <taylor@axfive.net>
 * This code is released under the license described in the LICENSE file
 */

#include "test_common.hxx"

#include <args.hxx>

#include "test_helpers.hxx"

int main()
{
    args::ArgumentParser parser("This is a test program.", "This goes after the options.");
    args::ValueFlag<unsigned int> uid(parser, "UID", "numeric id", {'u', "uid"});

    test::require_throws_as<args::ParseError>([&] { parser.ParseArgs(std::vector<std::string>{"--uid", "-1"}); });
    test::require_throws_as<args::ParseError>([&] { parser.ParseArgs(std::vector<std::string>{"--uid=-1"}); });
    test::require_throws_as<args::ParseError>([&] { parser.ParseArgs(std::vector<std::string>{"--uid", "123abc"}); });
    return 0;
}
