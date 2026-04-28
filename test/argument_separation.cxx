/* Copyright (c) Taylor Richberger <taylor@axfive.net>
 * This code is released under the license described in the LICENSE file
 */

#include "test_common.hxx"

#include <args.hxx>

#include "test_helpers.hxx"

int main()
{
    args::ArgumentParser parser("This is a test program.", "This goes after the options.");
    args::ValueFlag<std::string> bar(parser, "BAR", "test flag", {'b', "bar"});
    test::require_nothrow([&] { parser.ParseArgs(std::vector<std::string>{"-btest"}); });
    test::require_nothrow([&] { parser.ParseArgs(std::vector<std::string>{"--bar=test"}); });
    test::require_nothrow([&] { parser.ParseArgs(std::vector<std::string>{"-b", "test"}); });
    test::require_nothrow([&] { parser.ParseArgs(std::vector<std::string>{"--bar", "test"}); });
    parser.SetArgumentSeparations(true, false, false, false);
    test::require_nothrow([&] { parser.ParseArgs(std::vector<std::string>{"-btest"}); });
    test::require_throws_as<args::ParseError>([&] { parser.ParseArgs(std::vector<std::string>{"--bar=test"}); });
    test::require_throws_as<args::ParseError>([&] { parser.ParseArgs(std::vector<std::string>{"-b", "test"}); });
    test::require_throws_as<args::ParseError>([&] { parser.ParseArgs(std::vector<std::string>{"--bar", "test"}); });
    parser.SetArgumentSeparations(false, true, false, false);
    test::require_throws_as<args::ParseError>([&] { parser.ParseArgs(std::vector<std::string>{"-btest"}); });
    test::require_nothrow([&] { parser.ParseArgs(std::vector<std::string>{"--bar=test"}); });
    test::require_throws_as<args::ParseError>([&] { parser.ParseArgs(std::vector<std::string>{"-b", "test"}); });
    test::require_throws_as<args::ParseError>([&] { parser.ParseArgs(std::vector<std::string>{"--bar", "test"}); });
    parser.SetArgumentSeparations(false, false, true, false);
    test::require_throws_as<args::ParseError>([&] { parser.ParseArgs(std::vector<std::string>{"-btest"}); });
    test::require_throws_as<args::ParseError>([&] { parser.ParseArgs(std::vector<std::string>{"--bar=test"}); });
    test::require_nothrow([&] { parser.ParseArgs(std::vector<std::string>{"-b", "test"}); });
    test::require_throws_as<args::ParseError>([&] { parser.ParseArgs(std::vector<std::string>{"--bar", "test"}); });
    parser.SetArgumentSeparations(false, false, false, true);
    test::require_throws_as<args::ParseError>([&] { parser.ParseArgs(std::vector<std::string>{"-btest"}); });
    test::require_throws_as<args::ParseError>([&] { parser.ParseArgs(std::vector<std::string>{"--bar=test"}); });
    test::require_throws_as<args::ParseError>([&] { parser.ParseArgs(std::vector<std::string>{"-b", "test"}); });
    test::require_nothrow([&] { parser.ParseArgs(std::vector<std::string>{"--bar", "test"}); });
    return 0;
}
