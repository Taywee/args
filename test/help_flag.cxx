/* Copyright (c) Taylor Richberger <taylor@axfive.net>
 * This code is released under the license described in the LICENSE file
 */

#include "test_common.hxx"

#include <args.hxx>

#include "test_helpers.hxx"

int main()
{
    args::ArgumentParser parser("This is a test program.", "This goes after the options.");
    args::HelpFlag help(parser, "help", "Display this help menu", {'h', "help"});
    test::require_nothrow([&] { parser.ParseArgs(std::vector<std::string>{}); });
    test::require_throws_as<args::Help>([&] { parser.ParseArgs(std::vector<std::string>{"--help"}); });
    return 0;
}
