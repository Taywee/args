/* Copyright (c) Taylor Richberger <taylor@axfive.net>
 * This code is released under the license described in the LICENSE file
 */

#include "test_common.hxx"

#include <args.hxx>

#include "test_helpers.hxx"

int main()
{
    args::ArgumentParser parser("This is a test program.");
    args::ValueFlag<std::string> foo(parser, "FOO", "test flag", {'f', "foo"});
    args::Flag bar(parser, "BAR", "test flag", {'b', "bar"});

    const char *argv[] = {"prog", "-f", "test", "--bar"};
    const bool result = parser.ParseCLI(static_cast<int>(sizeof(argv) / sizeof(argv[0])), argv);

    test::require(result);
    test::require(foo);
    test::require(*foo == "test");
    test::require(bar);

    return 0;
}
