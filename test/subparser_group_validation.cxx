/* Copyright (c) Taylor Richberger <taylor@axfive.net>
 * This code is released under the license described in the LICENSE file
 */

#include "test_common.hxx"

#include <args.hxx>

#include "test_helpers.hxx"

int main()
{
    int x = 0;
    args::ArgumentParser p("parser");
    args::Command a(p, "a", "command a", [&](args::Subparser &s)
    {
        args::Group required(s, "", args::Group::Validators::All);
        args::ValueFlag<std::string> f(required, "", "", {'f'});
        s.Parse();
        ++x;
    });

    p.RequireCommand(false);
    test::require_nothrow([&] { p.ParseArgs(std::vector<std::string>{}); });
    test::require_nothrow([&] { p.ParseArgs(std::vector<std::string>{"a", "-f", "F"}); });
    test::require_throws_as<args::ValidationError>([&] { p.ParseArgs(std::vector<std::string>{"a"}); });
    test::require(x == 1);
    return 0;
}
