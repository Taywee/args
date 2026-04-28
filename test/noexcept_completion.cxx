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
    return 0;
}
