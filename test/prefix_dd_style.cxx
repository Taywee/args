/* Copyright (c) Taylor Richberger <taylor@axfive.net>
 * This code is released under the license described in the LICENSE file
 */

#include "test_common.hxx"

#include <args.hxx>

#include "test_helpers.hxx"

int main()
{
    args::ArgumentParser parser("This command likes to break your disks");
    parser.LongPrefix("");
    parser.LongSeparator("=");
    args::HelpFlag help(parser, "HELP", "Show this help menu.", {"help"});
    args::ValueFlag<long> bs(parser, "BYTES", "Block size", {"bs"}, 512);
    args::ValueFlag<long> skip(parser, "BYTES", "Bytes to skip", {"skip"}, 0);
    args::ValueFlag<std::string> input(parser, "BLOCK SIZE", "Block size", {"if"});
    args::ValueFlag<std::string> output(parser, "BLOCK SIZE", "Block size", {"of"});
    parser.ParseArgs(std::vector<std::string>{"skip=8", "if=/dev/null"});
    test::require_false(bs);
    test::require(*bs == 512);
    test::require(skip);
    test::require(*skip == 8);
    test::require(input);
    test::require(*input == "/dev/null");
    test::require_false(output);
    return 0;
}
