/* Copyright (c) Taylor Richberger <taylor@axfive.net>
 * This code is released under the license described in the LICENSE file
 */

#include "test_common.hxx"

#include <args.hxx>

#include "test_helpers.hxx"

int main()
{
    // Flags with an optional argument (Nargs min == 0) must honour
    // SetArgumentSeparations the same way a mandatory-argument ValueFlag does.
    // Previously the joined-argument check was skipped whenever nargs.min == 0,
    // so a disabled joined value slipped through for ImplicitValueFlag and
    // NargsValueFlag{0, k}.
    {
        args::ArgumentParser parser("This is a test program.");
        args::ImplicitValueFlag<int> jobs(parser, "N", "jobs", {'j', "jobs"}, 8, 0);
        parser.SetArgumentSeparations(true, false, true, true);
        test::require_throws_as<args::ParseError>([&] { parser.ParseArgs(std::vector<std::string>{"--jobs=7"}); });
        test::require_nothrow([&] { parser.ParseArgs(std::vector<std::string>{"--jobs"}); });
        test::require(*jobs == 8);
        test::require_nothrow([&] { parser.ParseArgs(std::vector<std::string>{"--jobs", "3"}); });
        test::require(*jobs == 3);
    }
    {
        args::ArgumentParser parser("This is a test program.");
        args::ImplicitValueFlag<int> jobs(parser, "N", "jobs", {'j', "jobs"}, 8, 0);
        parser.SetArgumentSeparations(false, true, true, true);
        test::require_throws_as<args::ParseError>([&] { parser.ParseArgs(std::vector<std::string>{"-j7"}); });
    }
    {
        args::ArgumentParser parser("This is a test program.");
        args::NargsValueFlag<std::string> nums(parser, "N", "nums", {'n', "nums"}, args::Nargs{0, 2});
        parser.SetArgumentSeparations(true, false, true, true);
        test::require_throws_as<args::ParseError>([&] { parser.ParseArgs(std::vector<std::string>{"--nums=5"}); });
    }
    // Joined values are still accepted when enabled (the default), so the fix
    // does not regress the normal optional-argument behaviour.
    {
        args::ArgumentParser parser("This is a test program.");
        args::ImplicitValueFlag<int> jobs(parser, "N", "jobs", {'j', "jobs"}, 8, 0);
        test::require_nothrow([&] { parser.ParseArgs(std::vector<std::string>{"--jobs=7"}); });
        test::require(*jobs == 7);
        test::require_nothrow([&] { parser.ParseArgs(std::vector<std::string>{"-j5"}); });
        test::require(*jobs == 5);
    }
    return 0;
}
