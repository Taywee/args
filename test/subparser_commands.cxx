/* Copyright (c) Taylor Richberger <taylor@axfive.net>
 * This code is released under the license described in the LICENSE file
 */

#include "test_common.hxx"

#include <args.hxx>

#include "test_helpers.hxx"

int main()
{
    args::Group globals;
    args::ValueFlag<std::string> gitdir(globals, "path", "", {"git-dir"});
    args::HelpFlag h(globals, "help", "help", {"help"});

    args::ArgumentParser p("git-like parser");
    args::GlobalOptions g(p, globals);

    std::vector<std::string> paths;

    args::Command add(p, "add", "Add file contents to the index", [&](args::Subparser &c)
    {
        args::PositionalList<std::string> pathsList(c, "paths", "files to add");
        c.Parse();
        paths.assign(std::begin(pathsList), std::end(pathsList));
    });

    args::Command commit(p, "commit", "record changes to the repository", [&](args::Subparser &c)
    {
        args::PositionalList<std::string> pathsList(c, "paths", "files to commit");
        c.Parse();
        paths.assign(std::begin(pathsList), std::end(pathsList));
    });

    p.RequireCommand(true);
    p.ParseArgs(std::vector<std::string>{"add", "--git-dir", "A", "B", "C", "D"});
    test::require(add);
    test::require(!commit);
    test::require((paths == std::vector<std::string>{"B", "C", "D"}));
    test::require(*gitdir == "A");
    return 0;
}
