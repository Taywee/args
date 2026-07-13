/* Copyright (c) Taylor Richberger <taylor@axfive.net>
 * This code is released under the license described in the LICENSE file
 */

#include "test_common.hxx"

#include <args.hxx>

#include "test_helpers.hxx"

int main()
{
    args::ArgumentParser p("parser");
    args::CompletionFlag c(p, {"completion"});
    args::Flag f(p, "foo", "description", {'f', "foo"});
    args::MapPositional<std::string, int> mp(p, "mp", "description", {{"alpha", 1}, {"beta", 2}});

    // Without a terminator both the positional choices and the flag are offered.
    test::require_throws_with([&] { p.ParseArgs(std::vector<std::string>{"--completion", "bash", "1", "test", ""}); }, "alpha\nbeta\n-f");

    // After the -- terminator the parser treats every following chunk as
    // positional, so only positional choices remain; flags are no longer valid
    // candidates and must not be suggested.
    test::require_throws_with([&] { p.ParseArgs(std::vector<std::string>{"--completion", "bash", "2", "test", "--", ""}); }, "alpha\nbeta");
    test::require_throws_with([&] { p.ParseArgs(std::vector<std::string>{"--completion", "bash", "2", "test", "--", "a"}); }, "alpha");
    test::require_throws_with([&] { p.ParseArgs(std::vector<std::string>{"--completion", "bash", "2", "test", "--", "-"}); }, "");
    test::require_throws_with([&] { p.ParseArgs(std::vector<std::string>{"--completion", "bash", "2", "test", "--", "--"}); }, "");

    args::ArgumentParser p2("parser");
    args::CompletionFlag c2(p2, {"completion"});
    args::Command command1(p2, "command1", "desc", [](args::Subparser &sp) { sp.Parse(); });
    args::Command command2(p2, "command2", "desc", [](args::Subparser &sp) { sp.Parse(); });

    // Commands are offered normally, but not once the terminator has been seen.
    test::require_throws_with([&] { p2.ParseArgs(std::vector<std::string>{"--completion", "bash", "1", "test", ""}); }, "command1\ncommand2");
    test::require_throws_with([&] { p2.ParseArgs(std::vector<std::string>{"--completion", "bash", "2", "test", "--", ""}); }, "");
    return 0;
}
