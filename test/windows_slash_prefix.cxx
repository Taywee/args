/* Copyright (c) Taylor Richberger <taylor@axfive.net>
 * This code is released under the license described in the LICENSE file
 */

#include "test_common.hxx"

#include <args.hxx>

#include "test_helpers.hxx"

int main()
{
    // Windows-style identical "/" short and long prefixes: /h must match the
    // short flag and /help the long one (#26).
    {
        args::ArgumentParser parser("This is a test program.");
        parser.ShortPrefix("/");
        parser.LongPrefix("/");
        parser.LongSeparator(":");
        args::Flag help(parser, "help", "display this help section", {'h', "help"});
        args::Flag version(parser, "version", "show the program version", {'v', "version"});
        args::ValueFlag<std::string> input(parser, "file", "the input file", {'i', "input"});
        parser.ParseArgs(std::vector<std::string>{"/h", "/v", "/i", "in.txt"});
        test::require(help);
        test::require(version);
        test::require(input);
        test::require(*input == "in.txt");
    }

    {
        args::ArgumentParser parser("This is a test program.");
        parser.ShortPrefix("/");
        parser.LongPrefix("/");
        parser.LongSeparator(":");
        args::Flag help(parser, "help", "display this help section", {'h', "help"});
        args::ValueFlag<std::string> input(parser, "file", "the input file", {'i', "input"});
        parser.ParseArgs(std::vector<std::string>{"/help", "/input:out.txt"});
        test::require(help);
        test::require(input);
        test::require(*input == "out.txt");
    }

    // Joined short values still go through ParseShort when prefixes match.
    {
        args::ArgumentParser parser("This is a test program.");
        parser.ShortPrefix("/");
        parser.LongPrefix("/");
        args::ValueFlag<std::string> input(parser, "file", "the input file", {'i', "input"});
        parser.ParseArgs(std::vector<std::string>{"/ifile.txt"});
        test::require(input);
        test::require(*input == "file.txt");
    }

    return 0;
}
