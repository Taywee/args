/* Copyright (c) Taylor Richberger <taylor@axfive.net>
 * This code is released under the license described in the LICENSE file
 */

#include "test_common.hxx"

#include <args.hxx>

#include "test_helpers.hxx"

int main()
{
    args::ArgumentParser p("git-like parser");
    args::Flag g(p, "GLOBAL", "global flag", {'g'}, args::Options::Global);

    args::Command add(p, "add", "add file contents to the index", [&](args::Subparser &c)
    {
        args::Flag flag(c, "FLAG", "flag", {'f'});
        c.Parse();
    });

    args::Command commit(p, "commit", "record changes to the repository", [&](args::Subparser &c)
    {
        args::Flag flag(c, "FLAG", "flag", {'f'});
        c.Parse();
    });

    p.Prog("git");
    p.RequireCommand(false);

    std::ostringstream s;

    auto d = p.GetDescription(p.helpParams, 0);
    s << p;
    test::require(s.str() == R"(  git [COMMAND] {OPTIONS}

    git-like parser

  OPTIONS:

      -g                                global flag
      add                               add file contents to the index
      commit                            record changes to the repository

)");

    p.ParseArgs(std::vector<std::string>{"add"});
    s.str("");
    s << p;
    test::require(s.str() == R"(  git add {OPTIONS}

    add file contents to the index

  OPTIONS:

      -f                                flag

)");

    p.ParseArgs(std::vector<std::string>{});
    s.str("");
    s << p;
    test::require(s.str() == R"(  git [COMMAND] {OPTIONS}

    git-like parser

  OPTIONS:

      -g                                global flag
      add                               add file contents to the index
      commit                            record changes to the repository

)");

    p.helpParams.showCommandChildren = true;
    p.ParseArgs(std::vector<std::string>{});
    s.str("");
    s << p;
    test::require(s.str() == R"(  git [COMMAND] {OPTIONS}

    git-like parser

  OPTIONS:

      -g                                global flag
      add                               add file contents to the index
        -f                                flag
      commit                            record changes to the repository
        -f                                flag

)");

    commit.Epilog("epilog");
    p.helpParams.showCommandFullHelp = true;
    p.ParseArgs(std::vector<std::string>{});
    s.str("");
    s << p;
    test::require(s.str() == R"(  git [COMMAND] {OPTIONS}

    git-like parser

  OPTIONS:

      -g                                global flag
      add {OPTIONS}

        add file contents to the index

        -f                                flag

      commit {OPTIONS}

        record changes to the repository

        -f                                flag

        epilog

)");
    return 0;
}
