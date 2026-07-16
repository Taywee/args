/* Copyright (c) Taylor Richberger <taylor@axfive.net>
 * This code is released under the license described in the LICENSE file
 */

#define ARGS_NOEXCEPT
#include "test_common.hxx"

#include <args.hxx>

#include "test_helpers.hxx"

int main()
{
    // A short flag shared by two options surfaces as a usage error.
    {
        args::ArgumentParser parser("test");
        args::Flag a(parser, "a", "", {'d', "arg1"});
        args::Flag b(parser, "b", "", {'d', "arg2"});
        parser.ParseArgs(std::vector<std::string>{});
        test::require(parser.GetError() == args::Error::Usage);
    }

    // And so does a shared long flag.
    {
        args::ArgumentParser parser("test");
        args::Flag a(parser, "a", "", {'a', "same"});
        args::Flag b(parser, "b", "", {'b', "same"});
        parser.ParseArgs(std::vector<std::string>{});
        test::require(parser.GetError() == args::Error::Usage);
    }

    // Unique matchers leave the parser error-free.
    {
        args::ArgumentParser parser("test");
        args::HelpFlag help(parser, "help", "", {'h', "help"});
        args::Flag verbose(parser, "verbose", "", {'v', "verbose"});
        parser.ParseArgs(std::vector<std::string>{"-v"});
        test::require(parser.GetError() == args::Error::None);
    }

    return 0;
}
