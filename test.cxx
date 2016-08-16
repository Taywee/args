/* Copyright © 2016 Taylor C. Richberger <taywee@gmx.com>
 * This code is released under the license described in the LICENSE file
 */

#include <tuple>
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
    args::HelpFlag help(parser, "help", "Display this help menu", {'h', "help"});
    REQUIRE_NOTHROW(parser.ParseArgs(std::vector<std::string>{}));
    REQUIRE_THROWS_AS(parser.ParseArgs(std::vector<std::string>{"--help"}), args::Help);
}

TEST_CASE("Unknown flags throw exceptions", "[args]")
{
    args::ArgumentParser parser("This is a test program.", "This goes after the options.");
    args::HelpFlag help(parser, "help", "Display this help menu", {'h', "help"});
    REQUIRE_NOTHROW(parser.ParseArgs(std::vector<std::string>{}));
    REQUIRE_THROWS_AS(parser.ParseArgs(std::vector<std::string>{"--Help"}), args::ParseError);
    REQUIRE_THROWS_AS(parser.ParseArgs(std::vector<std::string>{"-H"}), args::ParseError);
}

TEST_CASE("Boolean flags work as expected, with clustering", "[args]")
{
    args::ArgumentParser parser("This is a test program.", "This goes after the options.");
    args::Flag foo(parser, "FOO", "test flag", {'f', "foo"});
    args::Flag bar(parser, "BAR", "test flag", {'b', "bar"});
    args::Flag baz(parser, "BAZ", "test flag", {'a', "baz"});
    args::Flag bix(parser, "BAZ", "test flag", {'x', "bix"});
    parser.ParseArgs(std::vector<std::string>{"--baz", "-fb"});
    REQUIRE(foo);
    REQUIRE(bar);
    REQUIRE(baz);
    REQUIRE_FALSE(bix);
}

TEST_CASE("Count flag works as expected", "[args]")
{
    args::ArgumentParser parser("This is a test program.", "This goes after the options.");
    args::CounterFlag foo(parser, "FOO", "test flag", {'f', "foo"});
    args::CounterFlag bar(parser, "BAR", "test flag", {'b', "bar"}, 7);
    args::CounterFlag baz(parser, "BAZ", "test flag", {'z', "baz"}, 7);
    parser.ParseArgs(std::vector<std::string>{"--foo", "-fb", "--bar", "-b", "-f", "--foo"});
    REQUIRE(foo);
    REQUIRE(bar);
    REQUIRE_FALSE(baz);
    REQUIRE(args::get(foo) == 4);
    REQUIRE(args::get(bar) == 10);
    REQUIRE(args::get(baz) == 7);
}

TEST_CASE("Argument flags work as expected, with clustering", "[args]")
{
    args::ArgumentParser parser("This is a test program.", "This goes after the options.");
    args::ValueFlag<std::string> foo(parser, "FOO", "test flag", {'f', "foo"});
    args::Flag bar(parser, "BAR", "test flag", {'b', "bar"});
    args::ValueFlag<double> baz(parser, "BAZ", "test flag", {'a', "baz"});
    args::ValueFlag<char> bim(parser, "BAZ", "test flag", {'B', "bim"});
    args::Flag bix(parser, "BAZ", "test flag", {'x', "bix"});
    parser.ParseArgs(std::vector<std::string>{"-bftest", "--baz=7.555e2", "--bim", "c"});
    REQUIRE(foo);
    REQUIRE(args::get(foo) == "test");
    REQUIRE(bar);
    REQUIRE(baz);
    REQUIRE((args::get(baz) > 755.49 && args::get(baz) < 755.51));
    REQUIRE(bim);
    REQUIRE(args::get(bim) == 'c');
    REQUIRE_FALSE(bix);
}

TEST_CASE("Passing an argument to a non-argument flag throws an error", "[args]")
{
    args::ArgumentParser parser("This is a test program.", "This goes after the options.");
    args::Flag bar(parser, "BAR", "test flag", {'b', "bar"});
    REQUIRE_THROWS_AS(parser.ParseArgs(std::vector<std::string>{"--bar=test"}), args::ParseError);
}

TEST_CASE("Unified argument lists for match work", "[args]")
{
    args::ArgumentParser parser("This is a test program.", "This goes after the options.");
    args::ValueFlag<std::string> foo(parser, "FOO", "test flag", {'f', "foo"});
    args::Flag bar(parser, "BAR", "test flag", {"bar", 'b'});
    args::ValueFlag<double> baz(parser, "BAZ", "test flag", {'a', "baz"});
    args::ValueFlag<char> bim(parser, "BAZ", "test flag", {'B', "bim"});
    args::Flag bix(parser, "BAZ", "test flag", {"bix"});
    parser.ParseArgs(std::vector<std::string>{"-bftest", "--baz=7.555e2", "--bim", "c"});
    REQUIRE(foo);
    REQUIRE(args::get(foo) == "test");
    REQUIRE(bar);
    REQUIRE(baz);
    REQUIRE((args::get(baz) > 755.49 && args::get(baz) < 755.51));
    REQUIRE(bim);
    REQUIRE(args::get(bim) == 'c');
    REQUIRE_FALSE(bix);
}

TEST_CASE("Get can be assigned to for non-reference types", "[args]")
{
    args::ArgumentParser parser("This is a test program.", "This goes after the options.");
    args::ValueFlag<std::string> foo(parser, "FOO", "test flag", {'f', "foo"});
    parser.ParseArgs(std::vector<std::string>{"--foo=test"});
    REQUIRE(foo);
    REQUIRE(args::get(foo) == "test");
    args::get(foo) = "bar";
    REQUIRE(args::get(foo) == "bar");
}

TEST_CASE("Invalid argument parsing throws parsing exceptions", "[args]")
{
    args::ArgumentParser parser("This is a test program.", "This goes after the options.");
    args::ValueFlag<int> foo(parser, "FOO", "test flag", {'f', "foo"});
    REQUIRE_THROWS_AS(parser.ParseArgs(std::vector<std::string>{"--foo=7.5"}), args::ParseError);
    REQUIRE_THROWS_AS(parser.ParseArgs(std::vector<std::string>{"--foo", "7a"}), args::ParseError);
    REQUIRE_THROWS_AS(parser.ParseArgs(std::vector<std::string>{"--foo", "7e4"}), args::ParseError);
}

TEST_CASE("Argument flag lists work as expected", "[args]")
{
    args::ArgumentParser parser("This is a test program.", "This goes after the options.");
    args::ValueFlagList<int> foo(parser, "FOO", "test flag", {'f', "foo"});
    parser.ParseArgs(std::vector<std::string>{"--foo=7", "-f2", "-f", "9", "--foo", "42"});
    REQUIRE((args::get(foo) == std::vector<int>{7, 2, 9, 42}));
}

#include <unordered_set>

TEST_CASE("Argument flag lists work with sets", "[args]")
{
    args::ArgumentParser parser("This is a test program.", "This goes after the options.");
    args::ValueFlagList<std::string, std::unordered_set> foo(parser, "FOO", "test flag", {'f', "foo"});
    parser.ParseArgs(std::vector<std::string>{"--foo=7", "-fblah", "-f", "9", "--foo", "blah"});
    REQUIRE((args::get(foo) == std::unordered_set<std::string>{"7", "9", "blah"}));
}

TEST_CASE("Positional arguments and positional argument lists work as expected", "[args]")
{
    args::ArgumentParser parser("This is a test program.", "This goes after the options.");
    args::Positional<std::string> foo(parser, "FOO", "test flag");
    args::Positional<bool> bar(parser, "BAR", "test flag");
    args::PositionalList<char> baz(parser, "BAZ", "test flag");
    parser.ParseArgs(std::vector<std::string>{"this is a test flag", "0", "a", "b", "c", "x", "y", "z"});
    REQUIRE(foo);
    REQUIRE((args::get(foo) == "this is a test flag"));
    REQUIRE(bar);
    REQUIRE(!args::get(bar));
    REQUIRE(baz);
    REQUIRE((args::get(baz) == std::vector<char>{'a', 'b', 'c', 'x', 'y', 'z'}));
}

TEST_CASE("The option terminator works as expected", "[args]")
{
    args::ArgumentParser parser("This is a test program.", "This goes after the options.");
    args::Positional<std::string> foo(parser, "FOO", "test flag");
    args::Positional<bool> bar(parser, "BAR", "test flag");
    args::PositionalList<std::string> baz(parser, "BAZ", "test flag");
    args::Flag ofoo(parser, "FOO", "test flag", {'f', "foo"});
    args::Flag obar(parser, "BAR", "test flag", {"bar", 'b'});
    args::ValueFlag<double> obaz(parser, "BAZ", "test flag", {'a', "baz"});
    parser.ParseArgs(std::vector<std::string>{"--foo", "this is a test flag", "0", "a", "b", "--baz", "7.0", "c", "x", "y", "z"});
    REQUIRE(foo);
    REQUIRE((args::get(foo) == "this is a test flag"));
    REQUIRE(bar);
    REQUIRE(!args::get(bar));
    REQUIRE(baz);
    REQUIRE((args::get(baz) == std::vector<std::string>{"a", "b", "c", "x", "y", "z"}));
    REQUIRE(ofoo);
    REQUIRE(!obar);
    REQUIRE(obaz);
    parser.ParseArgs(std::vector<std::string>{"--foo", "this is a test flag", "0", "a", "--", "b", "--baz", "7.0", "c", "x", "y", "z"});
    REQUIRE(foo);
    REQUIRE((args::get(foo) == "this is a test flag"));
    REQUIRE(bar);
    REQUIRE(!args::get(bar));
    REQUIRE(baz);
    REQUIRE((args::get(baz) == std::vector<std::string>{"a", "b", "--baz", "7.0", "c", "x", "y", "z"}));
    REQUIRE(ofoo);
    REQUIRE(!obar);
    REQUIRE(!obaz);
    parser.ParseArgs(std::vector<std::string>{"--foo", "--", "this is a test flag", "0", "a", "b", "--baz", "7.0", "c", "x", "y", "z"});
    REQUIRE(foo);
    REQUIRE((args::get(foo) == "this is a test flag"));
    REQUIRE(bar);
    REQUIRE(!args::get(bar));
    REQUIRE(baz);
    REQUIRE((args::get(baz) == std::vector<std::string>{"a", "b", "--baz", "7.0", "c", "x", "y", "z"}));
    REQUIRE(ofoo);
    REQUIRE(!obar);
    REQUIRE(!obaz);
}

TEST_CASE("Positional lists work with sets", "[args]")
{
    args::ArgumentParser parser("This is a test program.", "This goes after the options.");
    args::PositionalList<std::string, std::unordered_set> foo(parser, "FOO", "test positional");
    parser.ParseArgs(std::vector<std::string>{"foo", "FoO", "bar", "baz", "foo", "9", "baz"});
    REQUIRE((args::get(foo) == std::unordered_set<std::string>{"foo", "FoO", "bar", "baz", "9"}));
}


TEST_CASE("Positionals that are unspecified evaluate false", "[args]")
{
    args::ArgumentParser parser("This is a test program.", "This goes after the options.");
    args::Positional<std::string> foo(parser, "FOO", "test flag");
    args::Positional<bool> bar(parser, "BAR", "test flag");
    args::PositionalList<char> baz(parser, "BAZ", "test flag");
    parser.ParseArgs(std::vector<std::string>{"this is a test flag again"});
    REQUIRE(foo);
    REQUIRE((args::get(foo) == "this is a test flag again"));
    REQUIRE_FALSE(bar);
    REQUIRE_FALSE(baz);
}

TEST_CASE("Additional positionals throw an exception", "[args]")
{
    args::ArgumentParser parser("This is a test program.", "This goes after the options.");
    args::Positional<std::string> foo(parser, "FOO", "test flag");
    args::Positional<bool> bar(parser, "BAR", "test flag");
    REQUIRE_THROWS_AS(parser.ParseArgs(std::vector<std::string>{"this is a test flag again", "1", "this has no positional available"}), args::ParseError);
}

TEST_CASE("Argument groups should throw when validation fails", "[args]")
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
    args::Flag a(xorgroup, "a", "test flag", {'a'});
    args::Flag b(xorgroup, "b", "test flag", {'b'});
    args::Flag c(xorgroup, "c", "test flag", {'c'});
    args::Group nxor(xorgroup, "this group provides all-or-none (nxor) validation", args::Group::Validators::AllOrNone);
    args::Flag d(nxor, "d", "test flag", {'d'});
    args::Flag e(nxor, "e", "test flag", {'e'});
    args::Flag f(nxor, "f", "test flag", {'f'});
    args::Group atleastone(xorgroup, "this group provides at-least-one validation", args::Group::Validators::AtLeastOne);
    args::Flag g(atleastone, "g", "test flag", {'g'});
    args::Flag h(atleastone, "h", "test flag", {'h'});
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

struct DoublesReader
{
    void operator()(const std::string &name, const std::string &value, std::tuple<double, double> &destination)
    {
        size_t commapos = 0;
        std::get<0>(destination) = std::stod(value, &commapos);
        std::get<1>(destination) = std::stod(std::string(value, commapos + 1));
    }
};

TEST_CASE("Custom types work", "[args]")
{
    {
        args::ArgumentParser parser("This is a test program.");
        args::Positional<std::tuple<int, int>> ints(parser, "INTS", "This takes a pair of integers.");
        args::Positional<std::tuple<double, double>, DoublesReader> doubles(parser, "DOUBLES", "This takes a pair of doubles.");
        REQUIRE_THROWS_AS(parser.ParseArgs(std::vector<std::string>{"1.2,2", "3.8,4"}), args::ParseError);
    }
    args::ArgumentParser parser("This is a test program.");
    args::Positional<std::tuple<int, int>> ints(parser, "INTS", "This takes a pair of integers.");
    args::Positional<std::tuple<double, double>, DoublesReader> doubles(parser, "DOUBLES", "This takes a pair of doubles.");
    parser.ParseArgs(std::vector<std::string>{"1,2", "3.8,4"});
    REQUIRE(std::get<0>(args::get(ints)) == 1);
    REQUIRE(std::get<1>(args::get(ints)) == 2);
    REQUIRE((std::get<0>(args::get(doubles)) > 3.79 && std::get<0>(args::get(doubles)) < 3.81));
    REQUIRE((std::get<1>(args::get(doubles)) > 3.99 && std::get<1>(args::get(doubles)) < 4.01));
}

TEST_CASE("Custom parser prefixes (dd-style)", "[args]")
{
    args::ArgumentParser parser("This command likes to break your disks");
    parser.LongPrefix("");
    parser.LongSeparator("=");
    args::HelpFlag help(parser, "HELP", "Show this help menu.", {"help"});
    args::ValueFlag<long> bs(parser, "BYTES", "Block size", {"bs"}, 512);
    args::ValueFlag<long> skip(parser, "BYTES", "Bytes to skip", {"skip"}, 0);
    args::ValueFlag<std::string> input(parser, "BLOCK SIZE", "Block size", {"if"});
    args::ValueFlag<std::string> output(parser, "BLOCK SIZE", "Block size", {"of"});
    parser.ParseArgs(std::vector<std::string>{"skip=8", "if=/dev/null"});
    REQUIRE_FALSE(bs);
    REQUIRE(args::get(bs) == 512);
    REQUIRE(skip);
    REQUIRE(args::get(skip) == 8);
    REQUIRE(input);
    REQUIRE(args::get(input) == "/dev/null");
    REQUIRE_FALSE(output);
}

TEST_CASE("Custom parser prefixes (Some Windows styles)", "[args]")
{
    args::ArgumentParser parser("This command likes to break your disks");
    parser.LongPrefix("/");
    parser.LongSeparator(":");
    args::HelpFlag help(parser, "HELP", "Show this help menu.", {"help"});
    args::ValueFlag<long> bs(parser, "BYTES", "Block size", {"bs"}, 512);
    args::ValueFlag<long> skip(parser, "BYTES", "Bytes to skip", {"skip"}, 0);
    args::ValueFlag<std::string> input(parser, "BLOCK SIZE", "Block size", {"if"});
    args::ValueFlag<std::string> output(parser, "BLOCK SIZE", "Block size", {"of"});
    parser.ParseArgs(std::vector<std::string>{"/skip:8", "/if:/dev/null"});
    REQUIRE_FALSE(bs);
    REQUIRE(args::get(bs) == 512);
    REQUIRE(skip);
    REQUIRE(args::get(skip) == 8);
    REQUIRE(input);
    REQUIRE(args::get(input) == "/dev/null");
    REQUIRE_FALSE(output);
}

TEST_CASE("Help menu can be grabbed as a string, passed into a stream, or by using the overloaded stream operator", "[args]")
{
    std::ostream null(nullptr);
    args::ArgumentParser parser("This command likes to break your disks");
    args::HelpFlag help(parser, "HELP", "Show this help menu.", {"help"});
    args::ValueFlag<long> bs(parser, "BYTES", "Block size", {"bs"}, 512);
    args::ValueFlag<long> skip(parser, "BYTES", "Bytes to skip", {"skip"}, 0);
    args::ValueFlag<std::string> input(parser, "BLOCK SIZE", "Block size", {"if"});
    args::ValueFlag<std::string> output(parser, "BLOCK SIZE", "Block size", {"of"});
    parser.ParseArgs(std::vector<std::string>{"--skip=8", "--if=/dev/null"});
    null << parser.Help();
    parser.Help(null);
    null << parser;
}

TEST_CASE("Required argument separation being violated throws an error", "[args]")
{
    args::ArgumentParser parser("This is a test program.", "This goes after the options.");
    args::ValueFlag<std::string> bar(parser, "BAR", "test flag", {'b', "bar"});
    REQUIRE_NOTHROW(parser.ParseArgs(std::vector<std::string>{"-btest"}));
    REQUIRE_NOTHROW(parser.ParseArgs(std::vector<std::string>{"--bar=test"}));
    REQUIRE_NOTHROW(parser.ParseArgs(std::vector<std::string>{"-b", "test"}));
    REQUIRE_NOTHROW(parser.ParseArgs(std::vector<std::string>{"--bar", "test"}));
    parser.SetArgumentSeparations(true, false, false, false);
    REQUIRE_NOTHROW(parser.ParseArgs(std::vector<std::string>{"-btest"}));
    REQUIRE_THROWS_AS(parser.ParseArgs(std::vector<std::string>{"--bar=test"}), args::ParseError);
    REQUIRE_THROWS_AS(parser.ParseArgs(std::vector<std::string>{"-b", "test"}), args::ParseError);
    REQUIRE_THROWS_AS(parser.ParseArgs(std::vector<std::string>{"--bar", "test"}), args::ParseError);
    parser.SetArgumentSeparations(false, true, false, false);
    REQUIRE_THROWS_AS(parser.ParseArgs(std::vector<std::string>{"-btest"}), args::ParseError);
    REQUIRE_NOTHROW(parser.ParseArgs(std::vector<std::string>{"--bar=test"}));
    REQUIRE_THROWS_AS(parser.ParseArgs(std::vector<std::string>{"-b", "test"}), args::ParseError);
    REQUIRE_THROWS_AS(parser.ParseArgs(std::vector<std::string>{"--bar", "test"}), args::ParseError);
    parser.SetArgumentSeparations(false, false, true, false);
    REQUIRE_THROWS_AS(parser.ParseArgs(std::vector<std::string>{"-btest"}), args::ParseError);
    REQUIRE_THROWS_AS(parser.ParseArgs(std::vector<std::string>{"--bar=test"}), args::ParseError);
    REQUIRE_NOTHROW(parser.ParseArgs(std::vector<std::string>{"-b", "test"}));
    REQUIRE_THROWS_AS(parser.ParseArgs(std::vector<std::string>{"--bar", "test"}), args::ParseError);
    parser.SetArgumentSeparations(false, false, false, true);
    REQUIRE_THROWS_AS(parser.ParseArgs(std::vector<std::string>{"-btest"}), args::ParseError);
    REQUIRE_THROWS_AS(parser.ParseArgs(std::vector<std::string>{"--bar=test"}), args::ParseError);
    REQUIRE_THROWS_AS(parser.ParseArgs(std::vector<std::string>{"-b", "test"}), args::ParseError);
    REQUIRE_NOTHROW(parser.ParseArgs(std::vector<std::string>{"--bar", "test"}));
}

enum class MappingEnum
{
    def,
    foo,
    bar,
    red,
    yellow,
    green
};

#include <unordered_map>
#include <algorithm>
#include <string>

struct ToLowerReader
{
    void operator()(const std::string &name, const std::string &value, std::string &destination)
    {
        destination = value;
        std::transform(destination.begin(), destination.end(), destination.begin(), ::tolower);
    }
};

TEST_CASE("Mapping types work as needed", "[args]")
{
    std::unordered_map<std::string, MappingEnum> map{
        {"default", MappingEnum::def},
        {"foo", MappingEnum::foo},
        {"bar", MappingEnum::bar},
        {"red", MappingEnum::red},
        {"yellow", MappingEnum::yellow},
        {"green", MappingEnum::green}};
    args::ArgumentParser parser("This is a test program.", "This goes after the options.");
    args::MapFlag<std::string, MappingEnum> dmf(parser, "DMF", "Maps string to an enum", {"dmf"}, map);
    args::MapFlag<std::string, MappingEnum> mf(parser, "MF", "Maps string to an enum", {"mf"}, map);
    args::MapFlag<std::string, MappingEnum, ToLowerReader> cimf(parser, "CIMF", "Maps string to an enum case-insensitively", {"cimf"}, map);
    args::MapFlagList<std::string, MappingEnum> mfl(parser, "MFL", "Maps string to an enum list", {"mfl"}, map);
    args::MapPositional<std::string, MappingEnum> mp(parser, "MP", "Maps string to an enum", map);
    args::MapPositionalList<std::string, MappingEnum> mpl(parser, "MPL", "Maps string to an enum list", map);
    parser.ParseArgs(std::vector<std::string>{"--mf=red", "--cimf=YeLLoW", "--mfl=bar", "foo", "--mfl=green", "red", "--mfl", "bar", "default"});
    REQUIRE_FALSE(dmf);
    REQUIRE(args::get(dmf) == MappingEnum::def);
    REQUIRE(mf);
    REQUIRE(args::get(mf) == MappingEnum::red);
    REQUIRE(cimf);
    REQUIRE(args::get(cimf) == MappingEnum::yellow);
    REQUIRE(mfl);
    REQUIRE((args::get(mfl) == std::vector<MappingEnum>{MappingEnum::bar, MappingEnum::green, MappingEnum::bar}));
    REQUIRE(mp);
    REQUIRE((args::get(mp) == MappingEnum::foo));
    REQUIRE(mpl);
    REQUIRE((args::get(mpl) == std::vector<MappingEnum>{MappingEnum::red, MappingEnum::def}));
    REQUIRE_THROWS_AS(parser.ParseArgs(std::vector<std::string>{"--mf=YeLLoW"}), args::MapError);
}

TEST_CASE("An exception should be thrown when a single-argument flag is matched multiple times and the constructor option is specified", "[args]")
{
    std::unordered_map<std::string, MappingEnum> map{
        {"default", MappingEnum::def},
        {"foo", MappingEnum::foo},
        {"bar", MappingEnum::bar},
        {"red", MappingEnum::red},
        {"yellow", MappingEnum::yellow},
        {"green", MappingEnum::green}};

    args::ArgumentParser parser("Test command");
    args::Flag foo(parser, "Foo", "Foo", {'f', "foo"}, true);
    args::ValueFlag<std::string> bar(parser, "Bar", "Bar", {'b', "bar"}, "", true);
    args::Flag bix(parser, "Bix", "Bix", {'x', "bix"});
    args::MapFlag<std::string, MappingEnum> baz(parser, "Baz", "Baz", {'B', "baz"}, map, MappingEnum::def, true);
    REQUIRE_THROWS_AS(parser.ParseArgs(std::vector<std::string>{"--foo", "-f", "-bblah"}), args::ExtraError);
    REQUIRE_NOTHROW(parser.ParseArgs(std::vector<std::string>{"--foo", "-xxx", "--bix", "-bblah", "--bix"}));
    REQUIRE_THROWS_AS(parser.ParseArgs(std::vector<std::string>{"--foo", "-bblah", "-blah"}), args::ExtraError);
    REQUIRE_THROWS_AS(parser.ParseArgs(std::vector<std::string>{"--foo", "-bblah", "--bar", "blah"}), args::ExtraError);
    REQUIRE_THROWS_AS(parser.ParseArgs(std::vector<std::string>{"--baz=red", "-B", "yellow"}), args::ExtraError);
    REQUIRE_THROWS_AS(parser.ParseArgs(std::vector<std::string>{"--baz", "red", "-Byellow"}), args::ExtraError);
    REQUIRE_NOTHROW(parser.ParseArgs(std::vector<std::string>{"--foo", "-Bgreen"}));
    REQUIRE(foo);
    REQUIRE_FALSE(bar);
    REQUIRE_FALSE(bix);
    REQUIRE(baz);
    REQUIRE(args::get(baz) == MappingEnum::green);
}

TEST_CASE("Sub-parsers should work through kick-out", "[args]")
{
    std::unordered_map<std::string, MappingEnum> map{
        {"default", MappingEnum::def},
        {"foo", MappingEnum::foo},
        {"bar", MappingEnum::bar},
        {"red", MappingEnum::red},
        {"yellow", MappingEnum::yellow},
        {"green", MappingEnum::green}};

    const std::vector<std::string> args{"--foo", "green", "--bar"};

    args::ArgumentParser parser1("Test command");
    args::Flag foo1(parser1, "Foo", "Foo", {'f', "foo"});
    args::Flag bar1(parser1, "Bar", "Bar", {'b', "bar"});
    args::MapPositional<std::string, MappingEnum> sub(parser1, "sub", "sub", map);
    sub.KickOut(true);

    auto next = parser1.ParseArgs(args);

    args::ArgumentParser parser2("Test command");
    args::Flag foo2(parser2, "Foo", "Foo", {'f', "foo"});
    args::Flag bar2(parser2, "Bar", "Bar", {'b', "bar"});

    parser2.ParseArgs(next, std::end(args));

    REQUIRE(foo1);
    REQUIRE_FALSE(bar1);
    REQUIRE(sub);
    REQUIRE(args::get(sub) == MappingEnum::green);
    REQUIRE_FALSE(foo2);
    REQUIRE(bar2);
}

TEST_CASE("Kick-out should work via all flags and value flags", "[args]")
{
    const std::vector<std::string> args{"-a", "-b", "--foo", "-ca", "--bar", "barvalue", "-db"};

    args::ArgumentParser parser1("Test command");
    args::Flag a1(parser1, "a", "a", {'a'});
    args::Flag b1(parser1, "b", "b", {'b'});
    args::Flag c1(parser1, "c", "c", {'c'});
    args::Flag d1(parser1, "d", "d", {'d'});
    args::Flag foo(parser1, "foo", "foo", {'f', "foo"});
    foo.KickOut(true);

    args::ArgumentParser parser2("Test command");
    args::Flag a2(parser2, "a", "a", {'a'});
    args::Flag b2(parser2, "b", "b", {'b'});
    args::Flag c2(parser2, "c", "c", {'c'});
    args::Flag d2(parser2, "d", "d", {'d'});
    args::ValueFlag<std::string> bar(parser2, "bar", "bar", {'B', "bar"});
    bar.KickOut(true);

    args::ArgumentParser parser3("Test command");
    args::Flag a3(parser3, "a", "a", {'a'});
    args::Flag b3(parser3, "b", "b", {'b'});
    args::Flag c3(parser3, "c", "c", {'c'});
    args::Flag d3(parser3, "d", "d", {'d'});

    auto next = parser1.ParseArgs(args);
    next = parser2.ParseArgs(next, std::end(args));
    next = parser3.ParseArgs(next, std::end(args));
    REQUIRE(next == std::end(args));
    REQUIRE(a1);
    REQUIRE(b1);
    REQUIRE_FALSE(c1);
    REQUIRE_FALSE(d1);
    REQUIRE(foo);
    REQUIRE(a2);
    REQUIRE_FALSE(b2);
    REQUIRE(c2);
    REQUIRE_FALSE(d2);
    REQUIRE(bar);
    REQUIRE(args::get(bar) == "barvalue");
    REQUIRE_FALSE(a3);
    REQUIRE(b3);
    REQUIRE_FALSE(c3);
    REQUIRE(d3);
}

#undef ARGS_HXX
#define ARGS_TESTNAMESPACE
#define ARGS_NOEXCEPT
#include <args.hxx>

TEST_CASE("Noexcept mode works as expected", "[args]")
{
    std::unordered_map<std::string, MappingEnum> map{
        {"default", MappingEnum::def},
        {"foo", MappingEnum::foo},
        {"bar", MappingEnum::bar},
        {"red", MappingEnum::red},
        {"yellow", MappingEnum::yellow},
        {"green", MappingEnum::green}};

    argstest::ArgumentParser parser("This is a test program.", "This goes after the options.");
    argstest::HelpFlag help(parser, "help", "Display this help menu", {'h', "help"});
    argstest::Flag bar(parser, "BAR", "test flag", {'b', "bar"}, true);
    argstest::ValueFlag<int> foo(parser, "FOO", "test flag", {'f', "foo"});
    argstest::Group nandgroup(parser, "this group provides nand validation", argstest::Group::Validators::AtMostOne);
    argstest::Flag x(nandgroup, "x", "test flag", {'x'});
    argstest::Flag y(nandgroup, "y", "test flag", {'y'});
    argstest::Flag z(nandgroup, "z", "test flag", {'z'});
    argstest::MapFlag<std::string, MappingEnum> mf(parser, "MF", "Maps string to an enum", {"mf"}, map);
    parser.ParseArgs(std::vector<std::string>{"-h"});
    REQUIRE(parser.GetError() == argstest::Error::Help);
    parser.ParseArgs(std::vector<std::string>{"--Help"});
    REQUIRE(parser.GetError() == argstest::Error::Parse);
    parser.ParseArgs(std::vector<std::string>{"--bar=test"});
    REQUIRE(parser.GetError() == argstest::Error::Parse);
    parser.ParseArgs(std::vector<std::string>{"--bar"});
    REQUIRE(parser.GetError() == argstest::Error::None);
    parser.ParseArgs(std::vector<std::string>{"--bar", "-b"});
    REQUIRE(parser.GetError() == argstest::Error::Extra);

    parser.ParseArgs(std::vector<std::string>{"--foo=7.5"});
    REQUIRE(parser.GetError() == argstest::Error::Parse);
    parser.ParseArgs(std::vector<std::string>{"--foo", "7a"});
    REQUIRE(parser.GetError() == argstest::Error::Parse);
    parser.ParseArgs(std::vector<std::string>{"--foo", "7e4"});
    REQUIRE(parser.GetError() == argstest::Error::Parse);
    parser.ParseArgs(std::vector<std::string>{"--foo"});
    REQUIRE(parser.GetError() == argstest::Error::Parse);

    parser.ParseArgs(std::vector<std::string>{"--foo=85"});
    REQUIRE(parser.GetError() == argstest::Error::None);

    parser.ParseArgs(std::vector<std::string>{"this is a test flag again", "1", "this has no positional available"});
    REQUIRE(parser.GetError() == argstest::Error::Parse);

    parser.ParseArgs(std::vector<std::string>{"-x"});
    REQUIRE(parser.GetError() == argstest::Error::None);
    parser.ParseArgs(std::vector<std::string>{"-xz"});
    REQUIRE(parser.GetError() == argstest::Error::Validation);
    parser.ParseArgs(std::vector<std::string>{"-y"});
    REQUIRE(parser.GetError() == argstest::Error::None);
    parser.ParseArgs(std::vector<std::string>{"-y", "-xz"});
    REQUIRE(parser.GetError() == argstest::Error::Validation);
    parser.ParseArgs(std::vector<std::string>{"--mf", "YeLLoW"});
    REQUIRE(parser.GetError() == argstest::Error::Map);
    parser.ParseArgs(std::vector<std::string>{"--mf", "yellow"});
    REQUIRE(parser.GetError() == argstest::Error::None);
}
