/* Copyright (c) Taylor Richberger <taylor@axfive.net>
 * This code is released under the license described in the LICENSE file
 */

#include "test_common.hxx"

#include <args.hxx>

#include "test_helpers.hxx"

int main()
{
    args::ArgumentParser p("git-like parser");

    std::vector<std::string> kickedOut;
    args::Command add(p, "add", "Add file contents to the index", [&](args::Subparser &c)
    {
        args::Flag kickoutFlag(c, "kick-out", "kick-out flag", {'k'}, args::Options::KickOut);
        c.Parse();
        test::require(kickoutFlag);
        kickedOut = c.KickedOut();
    });

    p.ParseArgs(std::vector<std::string>{"add", "-k", "A", "B", "C", "D"});
    test::require(add);
    test::require((kickedOut == std::vector<std::string>{"A", "B", "C", "D"}));
    return 0;
}
