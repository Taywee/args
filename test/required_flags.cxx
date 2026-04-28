/* Copyright (c) Taylor Richberger <taylor@axfive.net>
 * This code is released under the license described in the LICENSE file
 */

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

    test::require_throws_as<args::RequiredError>([&] { parser1.ParseArgs(std::vector<std::string>{"-b4"}); });

    args::ArgumentParser parser2("Test command");
    args::Positional<int> pos1(parser2, "a", "a");
    test::require_nothrow([&] { parser2.ParseArgs(std::vector<std::string>{}); });

    args::ArgumentParser parser3("Test command");
    args::Positional<int> pos2(parser3, "a", "a", args::Options::Required);
    test::require_throws_as<args::RequiredError>([&] { parser3.ParseArgs(std::vector<std::string>{}); });
    return 0;
}
