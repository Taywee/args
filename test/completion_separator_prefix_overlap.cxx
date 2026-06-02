/* Copyright (c) Taylor Richberger <taylor@axfive.net>
 * This code is released under the license described in the LICENSE file
 *
 * Regression test for an out-of-bounds std::string::substr in the bash
 * completion handler.
 *
 * When a custom LongSeparator overlaps the long prefix (here "-" under the
 * default "--" prefix), an attacker-controlled completion word such as
 * "--x" makes chunk.find(longseparator) land *inside* the prefix. The
 * handler then built `arg` (= chunk up to the separator) shorter than
 * longprefix and called arg.substr(longprefix.size()), which threw
 * std::out_of_range.
 *
 * That std::out_of_range is not an args::Error, so it escaped past the
 * documented `catch (args::Completion&) / catch (args::Error&)` idiom and
 * terminated the program. After the fix the joined-value completion is
 * simply skipped when the separator falls inside the prefix, so the normal
 * args::Completion path runs instead.
 */

#include "test_common.hxx"

#include <args.hxx>

#include "test_helpers.hxx"

int main()
{
    args::ArgumentParser p("parser");
    args::CompletionFlag c(p, {"completion"});
    args::ValueFlag<std::string> f(p, "f", "f", {'f', "foo"}, "abc");
    p.LongSeparator("-"); // legal, non-empty separator that overlaps "--"

    // Must throw args::Completion (the normal completion control-flow), not
    // a leaked std::out_of_range.
    test::require_throws_as<args::Completion>([&]
    {
        p.ParseArgs(std::vector<std::string>{"--completion", "bash", "1", "test", "--x"});
    });

    // A separator exactly at the prefix boundary leaves an empty flag name;
    // it must also complete cleanly rather than throw out_of_range.
    test::require_throws_as<args::Completion>([&]
    {
        p.ParseArgs(std::vector<std::string>{"--completion", "bash", "1", "test", "---x"});
    });

    return 0;
}
