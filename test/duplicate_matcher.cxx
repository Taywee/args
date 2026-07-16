/* Copyright (c) Taylor Richberger <taylor@axfive.net>
 * This code is released under the license described in the LICENSE file
 */

#include "test_common.hxx"

#include <args.hxx>

#include "test_helpers.hxx"

int main()
{
    // Two options that answer to the same short flag are ambiguous: the
    // second one can never be reached, so parsing should report it.
    test::require_throws_as<args::UsageError>([] {
        args::ArgumentParser parser("test");
        args::ValueFlag<std::string> a(parser, "a", "", {'d', "arg1"});
        args::ValueFlag<std::string> b(parser, "b", "", {'d', "arg2"});
        parser.ParseArgs(std::vector<std::string>{});
    });

    // The same holds when the shared flag is the long form.
    test::require_throws_as<args::UsageError>([] {
        args::ArgumentParser parser("test");
        args::Flag a(parser, "a", "", {'a', "same"});
        args::Flag b(parser, "b", "", {'b', "same"});
        parser.ParseArgs(std::vector<std::string>{});
    });

    // Distinct matchers, including multi-name aliases, must not be flagged.
    test::require_nothrow([] {
        args::ArgumentParser parser("test");
        args::HelpFlag help(parser, "help", "", {'h', "help"});
        args::Flag verbose(parser, "verbose", "", {'v', "verbose"});
        parser.ParseArgs(std::vector<std::string>{"-v"});
    });

    // Sibling commands may reuse a flag name; only one is ever reachable.
    test::require_nothrow([] {
        args::ArgumentParser parser("test");
        args::Command one(parser, "one", "");
        args::Flag vone(one, "v", "", {'v'});
        args::Command two(parser, "two", "");
        args::Flag vtwo(two, "v", "", {'v'});
        parser.ParseArgs(std::vector<std::string>{"one", "-v"});
    });

    // The same ambiguity inside a subcommand's own flags is caught too.
    test::require_throws_as<args::UsageError>([] {
        args::ArgumentParser parser("test");
        args::Command cmd(parser, "cmd", "", [](args::Subparser &s) {
            args::Flag x(s, "x", "", {'v'});
            args::Flag y(s, "y", "", {'v'});
            s.Parse();
        });
        parser.ParseArgs(std::vector<std::string>{"cmd"});
    });

    return 0;
}
