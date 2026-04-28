/* Copyright (c) Taylor Richberger <taylor@axfive.net>
 * This code is released under the license described in the LICENSE file
 *
 * Regression test for unsigned integer underflow in help formatting
 * when helpParams.width is smaller than the indentation/header overhead.
 * See commit 87bcccd.
 */

#include "test_common.hxx"

#include <args.hxx>

#include "test_helpers.hxx"

int main()
{
    args::ArgumentParser parser("Test program");
    args::Flag help(parser, "help", "Display help", {'h', "help"});

    parser.helpParams.width = 5;

    std::ostringstream out;
    test::require_nothrow([&] { out << parser; });
    return 0;
}
