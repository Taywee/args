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
    args::ValueFlag<int> a(parser, "", "", {});

    test::require(parser.GetError() == args::Error::Usage);
    parser.ParseArgs(std::vector<std::string>{"-a", "1", "2"});
    test::require(parser.GetError() == args::Error::Usage);
    return 0;
}
