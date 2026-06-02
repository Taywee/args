/* Copyright (c) Taylor Richberger <taylor@axfive.net>
 * This code is released under the license described in the LICENSE file
 *
 * ARGS_NOEXCEPT counterpart of completion_separator_prefix_overlap.
 *
 * std::string::substr throws std::out_of_range regardless of ARGS_NOEXCEPT,
 * so the unguarded arg.substr(longprefix.size()) in the bash completion
 * handler broke the no-throw contract: an ARGS_NOEXCEPT build (which may be
 * compiled with -fno-exceptions) would std::terminate on the completion
 * word "--x" when a custom LongSeparator overlaps the long prefix.
 *
 * After the fix ParseArgs returns normally and records the completion via
 * the error state instead of throwing.
 */

#define ARGS_NOEXCEPT
#include "test_common.hxx"

#include <args.hxx>

#include "test_helpers.hxx"

int main()
{
    args::ArgumentParser p("parser");
    args::CompletionFlag c(p, {"completion"});
    args::ValueFlag<std::string> f(p, "f", "f", {'f', "foo"}, "abc");
    p.LongSeparator("-");

    test::require_nothrow([&]
    {
        p.ParseArgs(std::vector<std::string>{"--completion", "bash", "1", "test", "--x"});
    });
    test::require(p.GetError() == args::Error::Completion);

    test::require_nothrow([&]
    {
        p.ParseArgs(std::vector<std::string>{"--completion", "bash", "1", "test", "---x"});
    });
    test::require(p.GetError() == args::Error::Completion);

    return 0;
}
