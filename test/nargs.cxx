/* Copyright (c) Taylor Richberger <taylor@axfive.net>
 * This code is released under the license described in the LICENSE file
 */

#include "test_common.hxx"

#include <args.hxx>

#include "test_helpers.hxx"

int main()
{
    args::ArgumentParser parser("Test command");
    args::NargsValueFlag<int> a(parser, "", "", {'a'}, 2);
    args::NargsValueFlag<int> b(parser, "", "", {'b'}, {2, 3});
    args::NargsValueFlag<std::string> c(parser, "", "", {'c'}, {0, 2});
    args::NargsValueFlag<int> d(parser, "", "", {'d'}, {1, 3});
    args::Flag f(parser, "", "", {'f'});

    test::require_throws_as<args::UsageError>([&] { args::Nargs(3, 2); });

    test::require_nothrow([&] { parser.ParseArgs(std::vector<std::string>{"-a", "1", "2"}); });
    test::require((*a == std::vector<int>{1, 2}));

    test::require_nothrow([&] { parser.ParseArgs(std::vector<std::string>{"-a", "1", "2", "-f"}); });
    test::require((*a == std::vector<int>{1, 2}));
    test::require(f);

    test::require_throws_as<args::ParseError>([&] { parser.ParseArgs(std::vector<std::string>{"-a", "1"}); });
    test::require_throws_as<args::ParseError>([&] { parser.ParseArgs(std::vector<std::string>{"-a1"}); });
    test::require_throws_as<args::ParseError>([&] { parser.ParseArgs(std::vector<std::string>{"-a1", "2"}); });

    test::require_nothrow([&] { parser.ParseArgs(std::vector<std::string>{"-b", "1", "-2", "-f"}); });
    test::require((*b == std::vector<int>{1, -2}));
    test::require(f);

    test::require_nothrow([&] { parser.ParseArgs(std::vector<std::string>{"-b", "1", "2", "3"}); });
    test::require((*b == std::vector<int>{1, 2, 3}));
    test::require(!f);

    std::vector<int> vec;
    for (int be : b)
    {
        vec.push_back(be);
    }

    test::require((vec == std::vector<int>{1, 2, 3}));
    vec.assign(std::begin(b), std::end(b));
    test::require((vec == std::vector<int>{1, 2, 3}));

    parser.SetArgumentSeparations(true, true, false, false);
    test::require_throws_as<args::ParseError>([&] { parser.ParseArgs(std::vector<std::string>{"-a", "1", "2"}); });

    test::require_nothrow([&] { parser.ParseArgs(std::vector<std::string>{"-c", "-f"}); });
    test::require(c->empty());
    test::require(f);

    test::require_nothrow([&] { parser.ParseArgs(std::vector<std::string>{"-cf"}); });
    test::require((*c == std::vector<std::string>{"f"}));
    test::require(!f);

    test::require_throws_as<args::ParseError>([&] { parser.ParseArgs(std::vector<std::string>{"-d"}); });
    test::require_throws_as<args::ParseError>([&] { parser.ParseArgs(std::vector<std::string>{"-b"}); });
    return 0;
}
