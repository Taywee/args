/* Copyright (c) Taylor Richberger <taylor@axfive.net>
 * This code is released under the license described in the LICENSE file
 */

#include "test_common.hxx"

#include <args.hxx>

#include "test_helpers.hxx"

int main()
{
    args::ArgumentParser p("parser");
    args::ValueFlag<std::string> f(p, "foo_name", "f", {"foo"});
    args::ValueFlag<std::string> g(p, "bar_name", "b", {"bar"});
    args::ValueFlag<std::string> z(p, "baz_name", "z", {"baz"});

    p.helpParams.proglineShowFlags = true;
    p.helpParams.width = 42;
    p.Prog("parser");
    p.ProglinePostfix("\na\nliiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiine line2 line2tail");

    test::require((p.GetProgramLine(p.helpParams) == std::vector<std::string>{
             "[--foo <foo_name>]",
             "[--bar <bar_name>]",
             "[--baz <baz_name>]",
             "\n",
             "a",
             "\n",
             "liiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiine",
             "line2",
             "line2tail",
             }));

    std::ostringstream s;
    s << p;
    test::require(s.str() == R"(  parser [--foo <foo_name>]
    [--bar <bar_name>]
    [--baz <baz_name>]
    a
    liiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiine
    line2 line2tail

    parser

  OPTIONS:

      --foo=[foo_name]                  f
      --bar=[bar_name]                  b
      --baz=[baz_name]                  z

)");
    return 0;
}
