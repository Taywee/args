/* Copyright (c) Taylor Richberger <taylor@axfive.net>
 * This code is released under the license described in the LICENSE file
 */

#define ARGS_NOEXCEPT
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

    p.ParseArgs(std::vector<std::string>{"--completion", "bash", "1", "test", "-"});
    test::require(p.GetError() == args::Error::Completion);
    test::require(args::get(c) == "-f\n-b");

    args::ArgumentParser p2("parser");
    args::CompletionFlag complete2(p2, {"completion"});

    args::Command c1(p2, "command1", "desc", [](args::Subparser &sp)
    {
        args::ValueFlag<std::string> f1(sp, "name", "description", {'f', "foo"}, "abc");
        sp.Parse();
    });

    args::Command c2(p2, "command2", "desc", [](args::Subparser &sp)
    {
        args::ValueFlag<std::string> f1(sp, "name", "description", {'b', "bar"}, "abc");
        sp.Parse();
    });

    p2.ParseArgs(std::vector<std::string>{"--completion", "bash", "2", "test", "command3", ""});
    test::require(p2.GetError() == args::Error::Completion);
    test::require(p2.GetErrorMsg().empty());
    test::require(args::get(complete2).empty());
    return 0;
}
