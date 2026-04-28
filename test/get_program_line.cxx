/* Copyright (c) Taylor Richberger <taylor@axfive.net>
 * This code is released under the license described in the LICENSE file
 */

#include "test_common.hxx"

#include <args.hxx>

#include "test_helpers.hxx"

int main()
{
    args::ArgumentParser p("parser");
    args::Flag g(p, "g", "g", {'g'}, args::Options::Global);
    args::Flag hidden(p, "hidden", "hidden flag", {'h'}, args::Options::Hidden);
    args::Command a(p, "a", "command a", [](args::Subparser &s)
    {
        args::ValueFlag<std::string> f(s, "STRING", "my f flag", {'f', "f-long"}, args::Options::Required);
        args::Positional<std::string> pos(s, "positional", "positional", args::Options::Required);
        s.Parse();
    });

    args::Command b(p, "b", "command b");
    args::ValueFlag<std::string> f(b, "STRING", "my f flag", {'f'}, args::Options::Required);
    args::Positional<std::string> pos(b, "positional", "positional");

    auto line = [&](args::Command &element)
    {
        p.Reset();
        auto strings = element.GetCommandProgramLine(p.helpParams);
        std::string res;
        for (const std::string &s: strings)
        {
            if (!res.empty())
            {
                res += ' ';
            }

            res += s;
        }

        return res;
    };

    test::require(line(p) == "COMMAND {OPTIONS}");
    test::require(line(a) == "a {OPTIONS} positional");
    test::require(line(b) == "b {OPTIONS} [positional]");

    p.helpParams.proglineShowFlags = true;
    test::require(line(p) == "COMMAND [-g]");
    test::require(line(a) == "a --f-long <STRING> positional");
    test::require(line(b) == "b -f <STRING> [positional]");

    p.helpParams.proglinePreferShortFlags = true;
    test::require(line(p) == "COMMAND [-g]");
    test::require(line(a) == "a -f <STRING> positional");
    test::require(line(b) == "b -f <STRING> [positional]");
    return 0;
}
