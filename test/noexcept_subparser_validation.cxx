/* Copyright (c) Taylor Richberger <taylor@axfive.net>
 * This code is released under the license described in the LICENSE file
 */

#define ARGS_NOEXCEPT
#include "test_common.hxx"

#include <args.hxx>

#include "test_helpers.hxx"

int main()
{
    args::ArgumentParser p("parser");
    args::Command a(p, "a", "command a", [](args::Subparser &s)
    {
        args::ValueFlag<std::string> f(s, "", "", {'f'}, args::Options::Required);
        s.Parse();
    });

    args::Command b(p, "b", "command b");
    args::ValueFlag<std::string> f(b, "", "", {'f'}, args::Options::Required);

    args::Command c(p, "c", "command c", [](args::Subparser&){});

    p.ParseArgs(std::vector<std::string>{});
    test::require(p.GetError() == args::Error::Validation);

    p.ParseArgs(std::vector<std::string>{"a"});
    test::require((size_t)p.GetError() == (size_t)args::Error::Required);

    p.ParseArgs(std::vector<std::string>{"a", "-f", "F"});
    test::require(p.GetError() == args::Error::None);

    p.ParseArgs(std::vector<std::string>{"b"});
    test::require(p.GetError() == args::Error::Required);

    p.ParseArgs(std::vector<std::string>{"b", "-f", "F"});
    test::require(p.GetError() == args::Error::None);

    p.RequireCommand(false);
    p.ParseArgs(std::vector<std::string>{});
    test::require(p.GetError() == args::Error::None);

    p.ParseArgs(std::vector<std::string>{"c"});
    test::require(p.GetError() == args::Error::Usage);
    return 0;
}
