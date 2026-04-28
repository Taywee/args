/* Copyright (c) Taylor Richberger <taylor@axfive.net>
 * This code is released under the license described in the LICENSE file
 */

#include "test_common.hxx"

#include <args.hxx>

#include "test_helpers.hxx"

int main()
{
    args::ArgumentParser parser("Test command");
    test::require_throws_as<args::UsageError>([&] { args::ValueFlag<int>(parser, "", "", {}); });
    return 0;
}
