/* Copyright (c) Taylor Richberger <taylor@axfive.net>
 * This code is released under the license described in the LICENSE file
 */

#include "test_common.hxx"

#include <args.hxx>

#include "test_helpers.hxx"

int main()
{
    args::Group globals;
    args::Flag f(globals, "f", "f", {'f'});

    args::ArgumentParser p("parser");
    args::GlobalOptions g(p, globals);
    args::Command a(p, "a", "command a");
    args::Command b(p, "b", "command b");

    p.RequireCommand(false);

    test::require_nothrow([&] { p.ParseArgs(std::vector<std::string>{"-f"}); });
    test::require_nothrow([&] { p.ParseArgs(std::vector<std::string>{"a", "-f"}); });
    test::require_nothrow([&] { p.ParseArgs(std::vector<std::string>{"b", "-f"}); });
    return 0;
}
