/* Copyright (c) Taylor Richberger <taylor@axfive.net>
 * This code is released under the license described in the LICENSE file
 *
 * Regression test for an out-of-bounds vector iterator increment that
 * occurred under ARGS_NOEXCEPT when bash completion fired on the value
 * position of a flag.
 *
 * Reproducer flow (without the fix):
 *  - Outer Parse enters the completion handler, builds curArgs = {"-m", ""},
 *    and recurses into Parse(curArgs.begin(), curArgs.end()).
 *  - Inner Parse dispatches "-m" to ParseShort, which calls ParseArgsValues
 *    to consume the (empty) value token.
 *  - Inside ParseArgsValues, Complete() returns true (no throw under
 *    ARGS_NOEXCEPT) and the handler sets `it = end`, then returns "".
 *  - ParseShort returns true to the inner Parse loop, which then executes
 *    `++it` on an already-end iterator -- undefined behavior, and the
 *    subsequent `*it` reads past the end of curArgs.
 *
 * Additionally, the inner Parse's `return Parse(curArgs.begin(),
 * curArgs.end())` propagates an iterator into the temporary curArgs back to
 * ParseCLI, where it is compared against std::end(args) after curArgs has
 * been destroyed -- a dangling-iterator comparison.
 */

#define ARGS_NOEXCEPT
#include "test_common.hxx"

#include <args.hxx>

#include "test_helpers.hxx"

int main()
{
    args::ArgumentParser p("parser");
    args::CompletionFlag c(p, {"completion"});
    args::MapFlag<std::string, int> m(p, "mappos", "mappos", {'m', "map"},
                                      {{"alpha", 1}, {"beta", 2}});

    // cword=2 lands on the empty value position of "-m". Without the fix,
    // the nested Parse advances its loop iterator past curArgs.end().
    p.ParseArgs(std::vector<std::string>{"--completion", "bash", "2", "test", "-m", ""});
    test::require(p.GetError() == args::Error::Completion);
    test::require(args::get(c) == "alpha\nbeta");

    // Long form via separate value token: nested Parse sees {"--map", ""}.
    p.ParseArgs(std::vector<std::string>{"--completion", "bash", "2", "test", "--map", ""});
    test::require(p.GetError() == args::Error::Completion);
    test::require(args::get(c) == "alpha\nbeta");

    // Exercise ParseCLI's iterator comparison: prior to the fix the
    // return value of ParseArgs pointed into the destroyed curArgs and the
    // == comparison against the outer args.end() in ParseCLI was a
    // dangling-iterator compare. Just confirm it terminates cleanly with
    // the expected completion state.
    const std::vector<const char *> argv{"prog", "--completion", "bash", "2", "test", "-m", ""};
    p.ParseCLI(static_cast<int>(argv.size()), argv.data());
    test::require(p.GetError() == args::Error::Completion);
    test::require(args::get(c) == "alpha\nbeta");

    return 0;
}
