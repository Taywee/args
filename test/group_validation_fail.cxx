/* Copyright (c) Taylor Richberger <taylor@axfive.net>
 * This code is released under the license described in the LICENSE file
 */

#include "test_common.hxx"

#include <args.hxx>

#include "test_helpers.hxx"

int main()
{
    args::ArgumentParser parser("This is a test program.", "This goes after the options.");
    args::Group xorgroup(parser, "this group provides xor validation", args::Group::Validators::Xor);
    args::Flag a(xorgroup, "a", "test flag", {'a'});
    args::Flag b(xorgroup, "b", "test flag", {'b'});
    args::Flag c(xorgroup, "c", "test flag", {'c'});
    args::Group nxor(parser, "this group provides all-or-none (nxor) validation", args::Group::Validators::AllOrNone);
    args::Flag d(nxor, "d", "test flag", {'d'});
    args::Flag e(nxor, "e", "test flag", {'e'});
    args::Flag f(nxor, "f", "test flag", {'f'});
    args::Group atleastone(parser, "this group provides at-least-one validation", args::Group::Validators::AtLeastOne);
    args::Flag g(atleastone, "g", "test flag", {'g'});
    args::Flag h(atleastone, "h", "test flag", {'h'});
    // Needs g or h
    test::require_throws_as<args::ValidationError>([&] { parser.ParseArgs(std::vector<std::string>{"-a"}); });
    test::require_nothrow([&] { parser.ParseArgs(std::vector<std::string>{"-g", "-a"}); });
    test::require_nothrow([&] { parser.ParseArgs(std::vector<std::string>{"-h", "-a"}); });
    test::require_nothrow([&] { parser.ParseArgs(std::vector<std::string>{"-gh", "-a"}); });
    // Xor stuff
    test::require_throws_as<args::ValidationError>([&] { parser.ParseArgs(std::vector<std::string>{"-g"}); });
    test::require_nothrow([&] { parser.ParseArgs(std::vector<std::string>{"-h", "-b"}); });
    test::require_throws_as<args::ValidationError>([&] { parser.ParseArgs(std::vector<std::string>{"-g", "-ab"}); });
    test::require_throws_as<args::ValidationError>([&] { parser.ParseArgs(std::vector<std::string>{"-g", "-ac"}); });
    test::require_throws_as<args::ValidationError>([&] { parser.ParseArgs(std::vector<std::string>{"-g", "-abc"}); });
    // Nxor stuff
    test::require_nothrow([&] { parser.ParseArgs(std::vector<std::string>{"-h", "-a"}); });
    test::require_nothrow([&] { parser.ParseArgs(std::vector<std::string>{"-h", "-adef"}); });
    test::require_throws_as<args::ValidationError>([&] { parser.ParseArgs(std::vector<std::string>{"-g", "-ad"}); });
    test::require_throws_as<args::ValidationError>([&] { parser.ParseArgs(std::vector<std::string>{"-g", "-adf"}); });
    test::require_throws_as<args::ValidationError>([&] { parser.ParseArgs(std::vector<std::string>{"-g", "-aef"}); });
    return 0;
}
