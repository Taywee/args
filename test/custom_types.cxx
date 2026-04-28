/* Copyright (c) Taylor Richberger <taylor@axfive.net>
 * This code is released under the license described in the LICENSE file
 */

#include "test_common.hxx"

#include <args.hxx>

#include "test_helpers.hxx"

int main()
{
    {
        args::ArgumentParser parser("This is a test program.");
        args::Positional<std::tuple<int, int>> ints(parser, "INTS", "This takes a pair of integers.");
        args::Positional<std::tuple<double, double>, DoublesReader> doubles(parser, "DOUBLES", "This takes a pair of doubles.");
        test::require_throws_as<args::ParseError>([&] { parser.ParseArgs(std::vector<std::string>{"1.2,2", "3.8,4"}); });
    }
    args::ArgumentParser parser("This is a test program.");
    args::Positional<std::tuple<int, int>> ints(parser, "INTS", "This takes a pair of integers.");
    args::Positional<std::tuple<double, double>, DoublesReader> doubles(parser, "DOUBLES", "This takes a pair of doubles.");
    parser.ParseArgs(std::vector<std::string>{"1,2", "3.8,4"});
    test::require(std::get<0>(*ints) == 1);
    test::require(std::get<1>(*ints) == 2);
    test::require((std::get<0>(*doubles) > 3.79 && std::get<0>(*doubles) < 3.81));
    test::require((std::get<1>(*doubles) > 3.99 && std::get<1>(*doubles) < 4.01));
    return 0;
}
