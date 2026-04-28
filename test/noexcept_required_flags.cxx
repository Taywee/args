/* Copyright (c) Taylor Richberger <taylor@axfive.net>
 * This code is released under the license described in the LICENSE file
 */

#define ARGS_NOEXCEPT
#include "test_common.hxx"

#include <args.hxx>

#include "test_helpers.hxx"

int main()
{
    args::ArgumentParser parser1("Test command");
    args::ValueFlag<int> foo(parser1, "foo", "foo", {'f', "foo"}, args::Options::Required);
    args::ValueFlag<int> bar(parser1, "bar", "bar", {'b', "bar"});

    parser1.ParseArgs(std::vector<std::string>{"-f", "42"});
    test::require(*foo == 42);
    test::require(parser1.GetError() == args::Error::None);
    test::require(parser1.GetErrorMsg() == std::string{});

    parser1.ParseArgs(std::vector<std::string>{"-b4"});
    test::require(parser1.GetError() == args::Error::Required);
    test::require(parser1.GetErrorMsg() != std::string{});

    args::ArgumentParser parser2("Test command");
    args::Positional<int> pos1(parser2, "a", "a");
    parser2.ParseArgs(std::vector<std::string>{});
    test::require(parser2.GetError() == args::Error::None);
    test::require(parser2.GetErrorMsg() == std::string{});

    args::ArgumentParser parser3("Test command");
    args::Positional<int> pos2(parser3, "a", "a", args::Options::Required);
    parser3.ParseArgs(std::vector<std::string>{});
    test::require(parser3.GetError() == args::Error::Required);
    test::require(parser3.GetErrorMsg() != std::string{});
    return 0;
}
