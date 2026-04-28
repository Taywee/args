/* Copyright (c) Taylor Richberger <taylor@axfive.net>
 * This code is released under the license described in the LICENSE file
 */

#include "test_common.hxx"

#include <args.hxx>

#include "test_helpers.hxx"

int main()
{
    args::ArgumentParser p("parser");
    args::Command a(p, "a", "command a", [](args::Subparser &s)
    {
        args::ValueFlag<std::string> f(s, "", "", {'f'}, args::Options::Required);
        s.Parse();
    });

    args::Command b(p, "b", "command b");
    args::ValueFlag<std::string> f(b, "", "", {'f'}, args::Options::Required);

    args::Command c(p, "c", "command c", [](args::Subparser&){});

    test::require_throws_as<args::ValidationError>([&] { p.ParseArgs(std::vector<std::string>{}); });
    test::require_throws_as<args::RequiredError>([&] { p.ParseArgs(std::vector<std::string>{"a"}); });
    test::require_nothrow([&] { p.ParseArgs(std::vector<std::string>{"a", "-f", "F"}); });
    test::require_throws_as<args::RequiredError>([&] { p.ParseArgs(std::vector<std::string>{"b"}); });
    test::require_nothrow([&] { p.ParseArgs(std::vector<std::string>{"b", "-f", "F"}); });

    p.RequireCommand(false);
    test::require_nothrow([&] { p.ParseArgs(std::vector<std::string>{}); });

    test::require_throws_as<args::UsageError>([&] { p.ParseArgs(std::vector<std::string>{"c"}); });

    test::require_throws_as<args::ParseError>([&] { p.ParseArgs(std::vector<std::string>{"unknown-command"}); });
    return 0;
}
