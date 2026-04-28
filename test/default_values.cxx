/* Copyright (c) Taylor Richberger <taylor@axfive.net>
 * This code is released under the license described in the LICENSE file
 */

#include "test_common.hxx"

#include <args.hxx>

#include "test_helpers.hxx"

int main()
{
    args::ArgumentParser p("parser");
    args::ValueFlag<std::string> f(p, "name", "description", {'f', "foo"}, "abc");
    args::MapFlag<std::string, int> b(p, "name", "description", {'b', "bar"}, {{"a", 1}, {"b", 2}, {"c", 3}});
    p.Prog("prog");
    test::require(p.Help() == R"(  prog {OPTIONS}

    parser

  OPTIONS:

      -f[name], --foo=[name]            description
      -b[name], --bar=[name]            description

)");

    p.helpParams.addDefault = true;
    p.helpParams.addChoices = true;

    test::require(p.Help() == R"(  prog {OPTIONS}

    parser

  OPTIONS:

      -f[name], --foo=[name]            description
                                        Default: abc
      -b[name], --bar=[name]            description
                                        One of: a, b, c

)");

    f.HelpDefault("123");
    b.HelpChoices({"1", "2", "3"});
    test::require(p.Help() == R"(  prog {OPTIONS}

    parser

  OPTIONS:

      -f[name], --foo=[name]            description
                                        Default: 123
      -b[name], --bar=[name]            description
                                        One of: 1, 2, 3

)");

    f.HelpDefault({});
    b.HelpChoices({});
    test::require(p.Help() == R"(  prog {OPTIONS}

    parser

  OPTIONS:

      -f[name], --foo=[name]            description
      -b[name], --bar=[name]            description

)");
    return 0;
}
