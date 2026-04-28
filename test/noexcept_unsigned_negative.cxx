/* Copyright (c) Taylor Richberger <taylor@axfive.net>
 * This code is released under the license described in the LICENSE file
 */

#define ARGS_NOEXCEPT
#include "test_common.hxx"

#include <args.hxx>

#include "test_helpers.hxx"

int main()
{
    args::ArgumentParser parser("Test command");
    args::ValueFlag<unsigned int> uid(parser, "UID", "numeric id", {'u', "uid"});

    parser.ParseArgs(std::vector<std::string>{"--uid", "-1"});
    test::require(parser.GetError() == args::Error::Parse);

    parser.ParseArgs(std::vector<std::string>{"--uid=-1"});
    test::require(parser.GetError() == args::Error::Parse);

    parser.ParseArgs(std::vector<std::string>{"--uid", "123abc"});
    test::require(parser.GetError() == args::Error::Parse);
    return 0;
}
