/* Copyright (c) Taylor Richberger <taylor@axfive.net>
 * This code is released under the license described in the LICENSE file
 */

#include "test_common.hxx"

#include <args.hxx>

#include "test_helpers.hxx"

int main()
{
    std::ostream null(nullptr);
    args::ArgumentParser parser("This command likes to break your disks");
    args::HelpFlag help(parser, "HELP", "Show this help menu.", {"help"});
    args::ValueFlag<long> bs(parser, "BYTES", "Block size", {"bs"}, 512);
    args::ValueFlag<long> skip(parser, "BYTES", "Bytes to skip", {"skip"}, 0);
    args::ValueFlag<std::string> input(parser, "BLOCK SIZE", "Block size", {"if"});
    args::ValueFlag<std::string> output(parser, "BLOCK SIZE", "Block size", {"of"});
    parser.ParseArgs(std::vector<std::string>{"--skip=8", "--if=/dev/null"});
    null << parser.Help();
    parser.Help(null);
    null << parser;
    return 0;
}
