/* Copyright (c) Taylor Richberger <taylor@axfive.net>
 * This code is released under the license described in the LICENSE file
 */

#include "test_common.hxx"

#include <args.hxx>

#include "test_helpers.hxx"

using IntFlag = args::ConstantFlag<int>;

int main()
{
    args::ArgumentParser parser("This is a test program.", "This goes after the options.");
    IntFlag flagA(parser, "A", "test flag", {'a', "flaga"}, 7);
    IntFlag flagB(parser, "B", "test flag". {'b', "flagb"}, 11);

    parser.ParseArgs(std::vector<std::string>{"-a"});

    auto matched = parser.GetMatchedChildren();
    test::require(matched.size() == 2);
    test::require(((IntFlag)matched[0]).Get() == 7);
    test::require(*((IntFlag)matched[0]) == 7);
    test::require(*((IntFlag)matched[1]).Get() == 11);
    test::require(*((IntFlag)matched[1]).Get() == 11);
    
    return 0;
}
