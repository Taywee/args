/* Copyright (c) Taylor Richberger <taylor@axfive.net>
 * This code is released under the license described in the LICENSE file
 */

#include "test_common.hxx"

#include <args.hxx>

#include "test_helpers.hxx"

int main()
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
    test::require(next == std::end(args));
    test::require(a1);
    test::require(b1);
    test::require_false(c1);
    test::require_false(d1);
    test::require(foo);
    test::require(a2);
    test::require_false(b2);
    test::require(c2);
    test::require_false(d2);
    test::require(bar);
    test::require(*bar == "barvalue");
    test::require_false(a3);
    test::require(b3);
    test::require_false(c3);
    test::require(d3);
    return 0;
}
