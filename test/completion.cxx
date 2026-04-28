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
    args::Group g(p);
    args::ValueFlag<std::string> f(g, "name", "description", {'f', "foo"}, "abc");
    args::ValueFlag<std::string> b(g, "name", "description", {'b', "bar"}, "abc");

    test::require_throws_with([&] { p.ParseArgs(std::vector<std::string>{"--completion", "bash", "1", "test", "-"}); }, "-f\n-b");
    test::require_throws_with([&] { p.ParseArgs(std::vector<std::string>{"--completion", "bash", "1", "test", "-f"}); }, "-f");
    test::require_throws_with([&] { p.ParseArgs(std::vector<std::string>{"--completion", "bash", "1", "test", "--"}); }, "--foo\n--bar");

    args::MapFlag<std::string, int> m(p, "mappos", "mappos", {'m', "map"}, {{"1",1}, {"2", 2}});
    test::require_throws_with([&] { p.ParseArgs(std::vector<std::string>{"--completion", "bash", "2", "test", "-m", ""}); }, "1\n2");
    test::require_throws_with([&] { p.ParseArgs(std::vector<std::string>{"--completion", "bash", "1", "test", "--map="}); }, "1\n2");
    test::require_throws_with([&] { p.ParseArgs(std::vector<std::string>{"--completion", "bash", "2", "test", "--map", "="}); }, "1\n2");
    test::require_throws_with([&] { p.ParseArgs(std::vector<std::string>{"--completion", "bash", "1", "test", "-m1"}); }, "-m1");

    args::Positional<std::string> pos(p, "name", "desc");
    test::require_throws_with([&] { p.ParseArgs(std::vector<std::string>{"--completion", "bash", "1", "test", ""}); }, "");
    test::require_throws_with([&] { p.ParseArgs(std::vector<std::string>{"--completion", "bash", "1", "test", "-"}); }, "-f\n-b\n-m");
    test::require_throws_with([&] { p.ParseArgs(std::vector<std::string>{"--completion", "bash", "1", "test", "--"}); }, "--foo\n--bar\n--map");

    args::ArgumentParser p2("parser");
    args::CompletionFlag complete2(p2, {"completion"});

    args::Command c1(p2, "command1", "desc", [](args::Subparser &sp)
    {
        args::ValueFlag<std::string> f1(sp, "name", "description", {'f', "foo"}, "abc");
        f1.KickOut();
        sp.Parse();
    });

    args::Command c2(p2, "command2", "desc", [](args::Subparser &sp)
    {
        args::ValueFlag<std::string> f1(sp, "name", "description", {'b', "bar"}, "abc");
        sp.Parse();
    });

    test::require_throws_with([&] { p2.ParseArgs(std::vector<std::string>{"--completion", "bash", "1", "test", "-"}); }, "");
    test::require_throws_with([&] { p2.ParseArgs(std::vector<std::string>{"--completion", "bash", "1", "test", ""}); }, "command1\ncommand2");
    test::require_throws_with([&] { p2.ParseArgs(std::vector<std::string>{"--completion", "bash", "2", "test", "command1", ""}); }, "-f");
    test::require_throws_with([&] { p2.ParseArgs(std::vector<std::string>{"--completion", "bash", "2", "test", "command2", ""}); }, "-b");
    test::require_throws_with([&] { p2.ParseArgs(std::vector<std::string>{"--completion", "bash", "2", "test", "command3", ""}); }, "");
    test::require_throws_with([&] { p2.ParseArgs(std::vector<std::string>{"--completion", "bash", "3", "test", "command1", "-f", "-"}); }, "");
    return 0;
}
