/* Copyright (c) Taylor Richberger <taylor@axfive.net>
 * This code is released under the license described in the LICENSE file
 */

#include "test_common.hxx"

#include <args.hxx>

#include "test_helpers.hxx"

int main()
{
    args::ArgumentParser p("parser");
    args::ValueFlag<std::string> f(p, "name", "description", {'f', "foo"});
    args::ValueFlag<std::string> g(p, "name", "description\n  d1\n  d2", {'g'});
    p.Prog("prog");

    test::require(p.Help() == R"(  prog {OPTIONS}

    parser

  OPTIONS:

      -f[name], --foo=[name]            description
      -g[name]                          description
                                          d1
                                          d2

)");

    p.helpParams.usageString = "usage:";
    p.helpParams.optionsString = "Options";
    p.helpParams.useValueNameOnce = true;
    test::require(p.Help() == R"(  usage: prog {OPTIONS}

    parser

  Options

      -f, --foo=[name]                  description
      -g[name]                          description
                                          d1
                                          d2

)");

    p.helpParams.showValueName = false;
    p.helpParams.optionsString = {};
    test::require(p.Help() == R"(  usage: prog {OPTIONS}

    parser

      -f, --foo                         description
      -g                                description
                                          d1
                                          d2

)");

    p.helpParams.helpindent = 12;
    p.helpParams.optionsString = "Options";
    test::require(p.Help() == R"(  usage: prog {OPTIONS}

    parser

  Options

      -f, --foo
            description
      -g    description
              d1
              d2

)");

    p.helpParams.addNewlineBeforeDescription = true;
    test::require(p.Help() == R"(  usage: prog {OPTIONS}

    parser

  Options

      -f, --foo
            description
      -g
            description
              d1
              d2

)");

    args::ValueFlag<std::string> e(p, "name", "some reaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaally loooooooooooooooooooooooooooong description", {'e'});
    test::require(p.Help() == R"(  usage: prog {OPTIONS}

    parser

  Options

      -f, --foo
            description
      -g
            description
              d1
              d2
      -e
            some reaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaally
            loooooooooooooooooooooooooooong description

)");
    return 0;
}
