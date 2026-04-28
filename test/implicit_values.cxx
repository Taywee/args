/* Copyright (c) Taylor Richberger <taylor@axfive.net>
 * This code is released under the license described in the LICENSE file
 */

#include "test_common.hxx"

#include <args.hxx>

#include "test_helpers.hxx"

int main()
{
    args::ArgumentParser parser("Test command");
    args::ImplicitValueFlag<int> j(parser, "parallel", "parallel", {'j', "parallel"}, 0, 1);
    args::Flag foo(parser, "FOO", "test flag", {'f', "foo"});
    test::require_nothrow([&] { parser.ParseArgs(std::vector<std::string>{"-j"}); });
    test::require(*j == 0);

    test::require_nothrow([&] { parser.ParseArgs(std::vector<std::string>{"-j4"}); });
    test::require(*j == 4);

    test::require_nothrow([&] { parser.ParseArgs(std::vector<std::string>{"-j", "4"}); });
    test::require(*j == 4);

    test::require_nothrow([&] { parser.ParseArgs(std::vector<std::string>{"-j", "-f"}); });
    test::require(*j == 0);
    test::require(foo);

    test::require_nothrow([&] { parser.ParseArgs(std::vector<std::string>{"-f"}); });
    test::require(*j == 1);
    test::require_false(j);
    return 0;
}
