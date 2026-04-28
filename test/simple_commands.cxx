/* Copyright (c) Taylor Richberger <taylor@axfive.net>
 * This code is released under the license described in the LICENSE file
 */

#include "test_common.hxx"

#include <args.hxx>

#include "test_helpers.hxx"

int main()
{
    args::ArgumentParser p("git-like parser");
    args::ValueFlag<std::string> gitdir(p, "path", "", {"git-dir"}, args::Options::Global);
    args::HelpFlag h(p, "help", "help", {"help"}, args::Options::Global);
    args::PositionalList<std::string> pathsList(p, "paths", "files to commit", args::Options::Global);
    args::Command add(p, "add", "Add file contents to the index");
    args::Command commit(p, "commit", "record changes to the repository");

    p.RequireCommand(true);
    p.ParseArgs(std::vector<std::string>{"add", "--git-dir", "A", "B", "C", "D"});
    test::require(add);
    test::require(!commit);
    test::require((*pathsList == std::vector<std::string>{"B", "C", "D"}));
    test::require(*gitdir == "A");
    return 0;
}
