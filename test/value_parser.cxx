/* Copyright (c) Taylor Richberger <taylor@axfive.net>
 * This code is released under the license described in the LICENSE file
 */

#include "test_common.hxx"

#include <args.hxx>

#include "test_helpers.hxx"

int main()
{
    static_assert(std::is_assignable<StringAssignable, std::string>::value, "StringAssignable must be assignable to std::string");

    args::ArgumentParser p("parser");
    args::ValueFlag<std::string> f(p, "name", "description", {'f'});
    args::ValueFlag<StringAssignable> b(p, "name", "description", {'b'});
    args::ValueFlag<int> i(p, "name", "description", {'i'});
    args::ValueFlag<int> d(p, "name", "description", {'d'});
    args::PositionalList<double> ds(p, "name", "description");

    test::require_nothrow([&] { p.ParseArgs(std::vector<std::string>{"-f", "a b"}); });
    test::require(*f == "a b");

    test::require_nothrow([&] { p.ParseArgs(std::vector<std::string>{"-b", "a b"}); });
    test::require(b->path == "a b");

    test::require_nothrow([&] { p.ParseArgs(std::vector<std::string>{"-i", "42 "}); });
    test::require(*i == 42);

    test::require_nothrow([&] { p.ParseArgs(std::vector<std::string>{"-i", " 12"}); });
    test::require(*i == 12);

    test::require_throws_as<args::ParseError>([&] { p.ParseArgs(std::vector<std::string>{"-i", "a"}); });
    test::require_throws_as<args::ParseError>([&] { p.ParseArgs(std::vector<std::string>{"-d", "b"}); });
    test::require_throws_as<args::ParseError>([&] { p.ParseArgs(std::vector<std::string>{"c"}); });
    test::require_throws_as<args::ParseError>([&] { p.ParseArgs(std::vector<std::string>{"s"}); });
    return 0;
}
