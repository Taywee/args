/* Copyright (c) Taylor Richberger <taylor@axfive.net>
 * This code is released under the license described in the LICENSE file
 */

#include "test_common.hxx"

#include <args.hxx>

#include "test_helpers.hxx"

int main()
{
    args::ArgumentParser p("parser");
    args::CompletionFlag c(p, {"completion"});
    args::Group g(p);
    args::ValueFlag<std::string> f(g, "name", "description", {'f', "foo"}, "abc");

    test::require_throws_as<args::ParseError>([&] { p.ParseArgs(std::vector<std::string>{"--completion", "bash", "1x", "test", "-"}); });
    test::require_throws_as<args::ParseError>([&] { p.ParseArgs(std::vector<std::string>{"--completion", "bash", "-1", "test", "-"}); });
    return 0;
}
