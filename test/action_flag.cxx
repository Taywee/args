/* Copyright (c) Taylor Richberger <taylor@axfive.net>
 * This code is released under the license described in the LICENSE file
 */

#include "test_common.hxx"

#include <args.hxx>

#include "test_helpers.hxx"

int main()
{
    args::ArgumentParser p("parser");
    std::string s;

    args::ActionFlag action0(p, "name", "description", {'x'}, [&]() { s = "flag"; });
    args::ActionFlag action1(p, "name", "description", {'y'}, [&](const std::string &arg) { s = arg; });
    args::ActionFlag actionN(p, "name", "description", {'z'}, 2, [&](const std::vector<std::string> &arg) { s = arg[0] + arg[1]; });
    args::ActionFlag actionThrow(p, "name", "description", {'v'}, [&]() { throw std::runtime_error(""); });

    p.ParseArgs(std::vector<std::string>{"-x"});
    test::require(s == "flag");

    p.ParseArgs(std::vector<std::string>{"-y", "a"});
    test::require(s == "a");

    p.ParseArgs(std::vector<std::string>{"-z", "a", "b"});
    test::require(s == "ab");

    test::require_throws_as<std::runtime_error>([&] { p.ParseArgs(std::vector<std::string>{"-v"}); });
    return 0;
}
