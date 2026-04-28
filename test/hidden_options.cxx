/* Copyright (c) Taylor Richberger <taylor@axfive.net>
 * This code is released under the license described in the LICENSE file
 */

#include "test_common.hxx"

#include <args.hxx>

#include "test_helpers.hxx"

int main()
{
    args::ArgumentParser parser1("");
    args::ValueFlag<int> foo(parser1, "foo", "foo", {'f', "foo"}, args::Options::HiddenFromDescription);
    args::ValueFlag<int> bar(parser1, "bar", "bar", {'b'}, args::Options::HiddenFromUsage);
    args::Group group(parser1, "group");
    args::ValueFlag<int> foo1(group, "foo", "foo", {'f', "foo"}, args::Options::Hidden);
    args::ValueFlag<int> bar2(group, "bar", "bar", {'b'});

    auto desc = parser1.GetDescription(parser1.helpParams, 0);
    test::require(desc.size() == 3);
    test::require(std::get<0>(desc[0]) == "-b[bar]");
    test::require(std::get<0>(desc[1]) == "group");
    test::require(std::get<0>(desc[2]) == "-b[bar]");

    parser1.helpParams.proglineShowFlags = true;
    parser1.helpParams.proglinePreferShortFlags = true;
    test::require((parser1.GetProgramLine(parser1.helpParams) == std::vector<std::string>{"[-f <foo>]", "[-b <bar>]"}));
    return 0;
}
