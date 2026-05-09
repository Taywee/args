/* Copyright (c) Taylor Richberger <taylor@axfive.net>
 * This code is released under the license described in the LICENSE file
 *
 * Regression test for AddCompletionReply substring offset.
 * Verifies bash completion of long-flag values with a multi-character
 * LongSeparator strips the entire separator (not just one byte).
 */

#include "test_common.hxx"

#include <args.hxx>

#include "test_helpers.hxx"

int main()
{
    args::ArgumentParser p("parser");
    args::CompletionFlag c(p, {"completion"});
    p.LongSeparator("==");
    args::MapFlag<std::string, int> m(p, "mappos", "mappos", {'m', "map"},
                                      {{"alpha", 1}, {"beta", 2}});

    test::require_throws_with(
        [&] { p.ParseArgs(std::vector<std::string>{"--completion", "bash", "1", "test", "--map=="}); },
        "alpha\nbeta");
    return 0;
}
