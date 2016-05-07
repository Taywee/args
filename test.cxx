/* Copyright © 2016 Taylor C. Richberger <taywee@gmx.com>
 * This code is released under the license described in the LICENSE file
 */

#include <iostream>

std::istream& operator>>(std::istream& is, std::tuple<int, int>& ints)
{
    is >> std::get<0>(ints);
    is.get();
    is >> std::get<1>(ints);
    return is;
}

#include <args.hxx>

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

TEST_CASE("Help flag throws Help exception", "[args]")
{
    args::ArgumentParser parser("This is a test program.", "This goes after the options.");
    args::HelpFlag help(parser, "help", "Display this help menu", args::Matcher({'h'}, {"help"}));
    REQUIRE_NOTHROW(parser.ParseArgs(std::vector<std::string>{}));
    REQUIRE_THROWS_AS(parser.ParseArgs(std::vector<std::string>{"--help"}), args::Help);
}

TEST_CASE("Unknown flags throw exceptions", "[args]")
{
    args::ArgumentParser parser("This is a test program.", "This goes after the options.");
    args::HelpFlag help(parser, "help", "Display this help menu", args::Matcher({'h'}, {"help"}));
    REQUIRE_NOTHROW(parser.ParseArgs(std::vector<std::string>{}));
    REQUIRE_THROWS_AS(parser.ParseArgs(std::vector<std::string>{"--Help"}), args::ParseError);
    REQUIRE_THROWS_AS(parser.ParseArgs(std::vector<std::string>{"-H"}), args::ParseError);
}

TEST_CASE("Boolean flags work as expected, with clustering", "[args]")
{
    args::ArgumentParser parser("This is a test program.", "This goes after the options.");
    args::Flag foo(parser, "FOO", "test flag", args::Matcher({'f'}, {"foo"}));
    args::Flag bar(parser, "BAR", "test flag", args::Matcher({'b'}, {"bar"}));
    args::Flag baz(parser, "BAZ", "test flag", args::Matcher({'a'}, {"baz"}));
    args::Flag bix(parser, "BAZ", "test flag", args::Matcher({'x'}, {"bix"}));
    parser.ParseArgs(std::vector<std::string>{"--baz", "-fb"});
    REQUIRE(foo);
    REQUIRE(bar);
    REQUIRE(baz);
    REQUIRE_FALSE(bix);
}

TEST_CASE("Argument flags work as expected, with clustering", "[args]")
{
    args::ArgumentParser parser("This is a test program.", "This goes after the options.");
    args::ArgFlag<std::string> foo(parser, "FOO", "test flag", args::Matcher({'f'}, {"foo"}));
    args::Flag bar(parser, "BAR", "test flag", args::Matcher({'b'}, {"bar"}));
    args::ArgFlag<double> baz(parser, "BAZ", "test flag", args::Matcher({'a'}, {"baz"}));
    args::ArgFlag<char> bim(parser, "BAZ", "test flag", args::Matcher({'B'}, {"bim"}));
    args::Flag bix(parser, "BAZ", "test flag", args::Matcher({'x'}, {"bix"}));
    parser.ParseArgs(std::vector<std::string>{"-bftest", "--baz=7.555e2", "--bim", "c"});
    REQUIRE(foo);
    REQUIRE(foo.value == "test");
    REQUIRE(bar);
    REQUIRE(baz);
    REQUIRE((baz.value > 755.49 && baz.value < 755.51));
    REQUIRE(bim);
    REQUIRE(bim.value == 'c');
    REQUIRE_FALSE(bix);
}

TEST_CASE("Unified argument lists for match work", "[args]")
{
    args::ArgumentParser parser("This is a test program.", "This goes after the options.");
    args::ArgFlag<std::string> foo(parser, "FOO", "test flag", args::Matcher{'f', "foo"});
    args::Flag bar(parser, "BAR", "test flag", args::Matcher{"bar", 'b'});
    args::ArgFlag<double> baz(parser, "BAZ", "test flag", args::Matcher{'a', "baz"});
    args::ArgFlag<char> bim(parser, "BAZ", "test flag", args::Matcher{'B', "bim"});
    args::Flag bix(parser, "BAZ", "test flag", args::Matcher{"bix"});
    parser.ParseArgs(std::vector<std::string>{"-bftest", "--baz=7.555e2", "--bim", "c"});
    REQUIRE(foo);
    REQUIRE(foo.value == "test");
    REQUIRE(bar);
    REQUIRE(baz);
    REQUIRE((baz.value > 755.49 && baz.value < 755.51));
    REQUIRE(bim);
    REQUIRE(bim.value == 'c');
    REQUIRE_FALSE(bix);
}

TEST_CASE("Invalid argument parsing throws parsing exceptions", "[args]")
{
    args::ArgumentParser parser("This is a test program.", "This goes after the options.");
    args::ArgFlag<int> foo(parser, "FOO", "test flag", args::Matcher({'f'}, {"foo"}));
    REQUIRE_THROWS_AS(parser.ParseArgs(std::vector<std::string>{"--foo=7.5"}), args::ParseError);
    REQUIRE_THROWS_AS(parser.ParseArgs(std::vector<std::string>{"--foo", "7a"}), args::ParseError);
    REQUIRE_THROWS_AS(parser.ParseArgs(std::vector<std::string>{"--foo", "7e4"}), args::ParseError);
}

TEST_CASE("Argument flag lists work as expected", "[args]")
{
    args::ArgumentParser parser("This is a test program.", "This goes after the options.");
    args::ArgFlagList<int> foo(parser, "FOO", "test flag", args::Matcher({'f'}, {"foo"}));
    parser.ParseArgs(std::vector<std::string>{"--foo=7", "-f2", "-f", "9", "--foo", "42"});
    REQUIRE((foo.values == std::vector<int>{7, 2, 9, 42}));
}

TEST_CASE("Positional arguments and positional argument lists work as expected", "[args]")
{
    args::ArgumentParser parser("This is a test program.", "This goes after the options.");
    args::PosArg<std::string> foo(parser, "FOO", "test flag");
    args::PosArg<bool> bar(parser, "BAR", "test flag");
    args::PosArgList<char> baz(parser, "BAZ", "test flag");
    parser.ParseArgs(std::vector<std::string>{"this is a test flag", "0", "a", "b", "c", "x", "y", "z"});
    REQUIRE(foo);
    REQUIRE((foo.value == "this is a test flag"));
    REQUIRE(bar);
    REQUIRE(!bar.value);
    REQUIRE(baz);
    REQUIRE((baz.values == std::vector<char>{'a', 'b', 'c', 'x', 'y', 'z'}));
}

TEST_CASE("Positionals that are unspecified evaluate false", "[args]")
{
    args::ArgumentParser parser("This is a test program.", "This goes after the options.");
    args::PosArg<std::string> foo(parser, "FOO", "test flag");
    args::PosArg<bool> bar(parser, "BAR", "test flag");
    args::PosArgList<char> baz(parser, "BAZ", "test flag");
    parser.ParseArgs(std::vector<std::string>{"this is a test flag again"});
    REQUIRE(foo);
    REQUIRE((foo.value == "this is a test flag again"));
    REQUIRE_FALSE(bar);
    REQUIRE_FALSE(baz);
}

TEST_CASE("Additional positionals throw an exception", "[args]")
{
    args::ArgumentParser parser("This is a test program.", "This goes after the options.");
    args::PosArg<std::string> foo(parser, "FOO", "test flag");
    args::PosArg<bool> bar(parser, "BAR", "test flag");
    REQUIRE_THROWS_AS(parser.ParseArgs(std::vector<std::string>{"this is a test flag again", "1", "this has no positional available"}), args::ParseError);
}

TEST_CASE("Argument groups should throw when validation fails", "[args]")
{
    args::ArgumentParser parser("This is a test program.", "This goes after the options.");
    args::Group xorgroup(parser, "this group provides xor validation", args::Group::Validators::Xor);
    args::Flag a(xorgroup, "a", "test flag", args::Matcher({'a'}));
    args::Flag b(xorgroup, "b", "test flag", args::Matcher({'b'}));
    args::Flag c(xorgroup, "c", "test flag", args::Matcher({'c'}));
    args::Group nxor(parser, "this group provides all-or-none (nxor) validation", args::Group::Validators::AllOrNone);
    args::Flag d(nxor, "d", "test flag", args::Matcher({'d'}));
    args::Flag e(nxor, "e", "test flag", args::Matcher({'e'}));
    args::Flag f(nxor, "f", "test flag", args::Matcher({'f'}));
    args::Group atleastone(parser, "this group provides at-least-one validation", args::Group::Validators::AtLeastOne);
    args::Flag g(atleastone, "g", "test flag", args::Matcher({'g'}));
    args::Flag h(atleastone, "h", "test flag", args::Matcher({'h'}));
    // Needs g or h
    REQUIRE_THROWS_AS(parser.ParseArgs(std::vector<std::string>{"-a"}), args::ValidationError);
    REQUIRE_NOTHROW(parser.ParseArgs(std::vector<std::string>{"-g", "-a"}));
    REQUIRE_NOTHROW(parser.ParseArgs(std::vector<std::string>{"-h", "-a"}));
    REQUIRE_NOTHROW(parser.ParseArgs(std::vector<std::string>{"-gh", "-a"}));
    // Xor stuff
    REQUIRE_THROWS_AS(parser.ParseArgs(std::vector<std::string>{"-g"}), args::ValidationError);
    REQUIRE_NOTHROW(parser.ParseArgs(std::vector<std::string>{"-h", "-b"}));
    REQUIRE_THROWS_AS(parser.ParseArgs(std::vector<std::string>{"-g", "-ab"}), args::ValidationError);
    REQUIRE_THROWS_AS(parser.ParseArgs(std::vector<std::string>{"-g", "-ac"}), args::ValidationError);
    REQUIRE_THROWS_AS(parser.ParseArgs(std::vector<std::string>{"-g", "-abc"}), args::ValidationError);
    // Nxor stuff
    REQUIRE_NOTHROW(parser.ParseArgs(std::vector<std::string>{"-h", "-a"}));
    REQUIRE_NOTHROW(parser.ParseArgs(std::vector<std::string>{"-h", "-adef"}));
    REQUIRE_THROWS_AS(parser.ParseArgs(std::vector<std::string>{"-g", "-ad"}), args::ValidationError);
    REQUIRE_THROWS_AS(parser.ParseArgs(std::vector<std::string>{"-g", "-adf"}), args::ValidationError);
    REQUIRE_THROWS_AS(parser.ParseArgs(std::vector<std::string>{"-g", "-aef"}), args::ValidationError);
}

TEST_CASE("Argument groups should nest", "[args]")
{
    args::ArgumentParser parser("This is a test program.", "This goes after the options.");
    args::Group xorgroup(parser, "this group provides xor validation", args::Group::Validators::Xor);
    args::Flag a(xorgroup, "a", "test flag", args::Matcher({'a'}));
    args::Flag b(xorgroup, "b", "test flag", args::Matcher({'b'}));
    args::Flag c(xorgroup, "c", "test flag", args::Matcher({'c'}));
    args::Group nxor(xorgroup, "this group provides all-or-none (nxor) validation", args::Group::Validators::AllOrNone);
    args::Flag d(nxor, "d", "test flag", args::Matcher({'d'}));
    args::Flag e(nxor, "e", "test flag", args::Matcher({'e'}));
    args::Flag f(nxor, "f", "test flag", args::Matcher({'f'}));
    args::Group atleastone(xorgroup, "this group provides at-least-one validation", args::Group::Validators::AtLeastOne);
    args::Flag g(atleastone, "g", "test flag", args::Matcher({'g'}));
    args::Flag h(atleastone, "h", "test flag", args::Matcher({'h'}));
    // Nothing actually matches, because nxor validates properly when it's empty, 
    REQUIRE_NOTHROW(parser.ParseArgs(std::vector<std::string>{}));
    REQUIRE_NOTHROW(parser.ParseArgs(std::vector<std::string>{"-a", "-d"}));
    REQUIRE_NOTHROW(parser.ParseArgs(std::vector<std::string>{"-c", "-f"}));
    REQUIRE_NOTHROW(parser.ParseArgs(std::vector<std::string>{"-de", "-f"}));
    REQUIRE_NOTHROW(parser.ParseArgs(std::vector<std::string>{"-gh", "-f"}));
    REQUIRE_THROWS_AS(parser.ParseArgs(std::vector<std::string>{"-g"}), args::ValidationError);
    REQUIRE_THROWS_AS(parser.ParseArgs(std::vector<std::string>{"-a"}), args::ValidationError);
    REQUIRE_THROWS_AS(parser.ParseArgs(std::vector<std::string>{"-b"}), args::ValidationError);
    REQUIRE_THROWS_AS(parser.ParseArgs(std::vector<std::string>{"-a", "-dg"}), args::ValidationError);
}

#include <tuple>

void DoublesReader(const std::string &name, const std::string &value, std::tuple<double, double> &destination)
{
    size_t commapos = 0;
    std::get<0>(destination) = std::stod(value, &commapos);
    std::get<1>(destination) = std::stod(std::string(value, commapos + 1));
}

TEST_CASE("Custom types work", "[args]")
{
    {
        args::ArgumentParser parser("This is a test program.");
        args::PosArg<std::tuple<int, int>> ints(parser, "INTS", "This takes a pair of integers.");
        args::PosArg<std::tuple<double, double>, DoublesReader> doubles(parser, "DOUBLES", "This takes a pair of doubles.");
        REQUIRE_THROWS_AS(parser.ParseArgs(std::vector<std::string>{"1.2,2", "3.8,4"}), args::ParseError);
    }
    args::ArgumentParser parser("This is a test program.");
    args::PosArg<std::tuple<int, int>> ints(parser, "INTS", "This takes a pair of integers.");
    args::PosArg<std::tuple<double, double>, DoublesReader> doubles(parser, "DOUBLES", "This takes a pair of doubles.");
    parser.ParseArgs(std::vector<std::string>{"1,2", "3.8,4"});
    REQUIRE(std::get<0>(ints.value) == 1);
    REQUIRE(std::get<1>(ints.value) == 2);
    REQUIRE((std::get<0>(doubles.value) > 3.79 && std::get<0>(doubles.value) < 3.81));
    REQUIRE((std::get<1>(doubles.value) > 3.99 && std::get<1>(doubles.value) < 4.01));
}

TEST_CASE("Custom parser prefixes (dd-style)", "[args]")
{
    args::ArgumentParser parser("This command likes to break your disks");
    parser.LongPrefix("");
    parser.LongSeparator("=");
    args::HelpFlag help(parser, "HELP", "Show this help menu.", args::Matcher({"help"}));
    args::ArgFlag<long> bs(parser, "BYTES", "Block size", args::Matcher({"bs"}), 512);
    args::ArgFlag<long> skip(parser, "BYTES", "Bytes to skip", args::Matcher({"skip"}), 0);
    args::ArgFlag<std::string> input(parser, "BLOCK SIZE", "Block size", args::Matcher({"if"}));
    args::ArgFlag<std::string> output(parser, "BLOCK SIZE", "Block size", args::Matcher({"of"}));
    parser.ParseArgs(std::vector<std::string>{"skip=8", "if=/dev/null"});
    REQUIRE_FALSE(bs);
    REQUIRE(bs.value == 512);
    REQUIRE(skip);
    REQUIRE(skip.value == 8);
    REQUIRE(input);
    REQUIRE(input.value == "/dev/null");
    REQUIRE_FALSE(output);
}

TEST_CASE("Custom parser prefixes (Some Windows styles)", "[args]")
{
    args::ArgumentParser parser("This command likes to break your disks");
    parser.LongPrefix("/");
    parser.LongSeparator(":");
    args::HelpFlag help(parser, "HELP", "Show this help menu.", args::Matcher({"help"}));
    args::ArgFlag<long> bs(parser, "BYTES", "Block size", args::Matcher({"bs"}), 512);
    args::ArgFlag<long> skip(parser, "BYTES", "Bytes to skip", args::Matcher({"skip"}), 0);
    args::ArgFlag<std::string> input(parser, "BLOCK SIZE", "Block size", args::Matcher({"if"}));
    args::ArgFlag<std::string> output(parser, "BLOCK SIZE", "Block size", args::Matcher({"of"}));
    parser.ParseArgs(std::vector<std::string>{"/skip:8", "/if:/dev/null"});
    REQUIRE_FALSE(bs);
    REQUIRE(bs.value == 512);
    REQUIRE(skip);
    REQUIRE(skip.value == 8);
    REQUIRE(input);
    REQUIRE(input.value == "/dev/null");
    REQUIRE_FALSE(output);
}

TEST_CASE("Help menu can be grabbed as a string, passed into a stream, or by using the overloaded stream operator", "[args]")
{
    std::ostream null(nullptr);
    args::ArgumentParser parser("This command likes to break your disks");
    args::HelpFlag help(parser, "HELP", "Show this help menu.", args::Matcher({"help"}));
    args::ArgFlag<long> bs(parser, "BYTES", "Block size", args::Matcher({"bs"}), 512);
    args::ArgFlag<long> skip(parser, "BYTES", "Bytes to skip", args::Matcher({"skip"}), 0);
    args::ArgFlag<std::string> input(parser, "BLOCK SIZE", "Block size", args::Matcher({"if"}));
    args::ArgFlag<std::string> output(parser, "BLOCK SIZE", "Block size", args::Matcher({"of"}));
    parser.ParseArgs(std::vector<std::string>{"--skip=8", "--if=/dev/null"});
    null << parser.Help();
    parser.Help(null);
    null << parser;
}
