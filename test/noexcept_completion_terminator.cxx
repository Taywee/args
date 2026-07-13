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
    args::Flag f(p, "foo", "description", {'f', "foo"});
    args::MapPositional<std::string, int> mp(p, "mp", "description", {{"alpha", 1}, {"beta", 2}});

    p.ParseArgs(std::vector<std::string>{"--completion", "bash", "1", "test", ""});
    test::require(p.GetError() == args::Error::Completion);
    test::require(args::get(c) == "alpha\nbeta\n-f");

    // After the -- terminator only positional choices remain; flags are not
    // offered because the parser would treat the chunk as positional.
    p.ParseArgs(std::vector<std::string>{"--completion", "bash", "2", "test", "--", ""});
    test::require(p.GetError() == args::Error::Completion);
    test::require(args::get(c) == "alpha\nbeta");

    p.ParseArgs(std::vector<std::string>{"--completion", "bash", "2", "test", "--", "-"});
    test::require(p.GetError() == args::Error::Completion);
    test::require(args::get(c).empty());

    return 0;
}
