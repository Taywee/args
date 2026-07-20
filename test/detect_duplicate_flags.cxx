/* Copyright (c) Taylor Richberger <taylor@axfive.net>
 * This code is released under the license described in the LICENSE file
 */

#include <args.hxx>

#include "test_helpers.hxx"

void testDuplicateShort()
{
    args::ArgumentParser parser("This is a test program.", "This goes after the options.");
    args::Flag flag_aone(parser, "aone", "test flag", {'a', "aone"});
    test::require_throws_as<args::ParseError>([&]{
        args::Flag flag_atwo(parser, "atwo", "test flag", {'a', "atwo"});
    });
}

void testDuplicateLong()
{
    args::ArgumentParser parser("This is a test program.", "This goes after the options.");
    args::Flag flag_a(parser, "aflag", "test flag", {'a', "flag"});
    test::require_throws_as<args::ParseError>([&]{
        args::Flag flag_b(parser, "bflag", "test flag", {'b', "flag"});
    });
}

void testDuplicateShortInGroup()
{
    args::ArgumentParser parser("This is a test program.", "This goes after the options.");
    args::Group group(parser, "This is a test group.", args::Group::Validators::DontCare);
    args::Flag flag_aone(group, "aone", "test flag", {'a', "aone"});
    test::require_throws_as<args::ParseError>([&]{
        args::Flag flag_atwo(group, "atwo", "test flag", {'a', "atwo"});
    });
}

void testDuplicateLongInGroup()
{
    args::ArgumentParser parser("This is a test program.", "This goes after the options.");
    args::Group group(parser, "This is a test group.", args::Group::Validators::DontCare);
    args::Flag flag_a(group, "aflag", "test flag", {'a', "flag"});
    test::require_throws_as<args::ParseError>([&]{
        args::Flag flag_b(group, "bflag", "test flag", {'b', "flag"});
    });
}

void testDuplicateShortInTwoGroups()
{
    args::ArgumentParser parser("This is a test program.", "This goes after the options.");
    args::Group group1(parser, "This is a test group.", args::Group::Validators::DontCare);
    args::Flag flag_a(group1, "a", "test flag", {'a', "flaga"});
    args::Flag flag_b(group1, "b", "test flag", {'b', "flagb"});
    args::Group group2(parser, "This is a test group.", args::Group::Validators::DontCare);
    args::Flag flag_c(group2, "c", "test flag", {'c', "flagc"});
    test::require_throws_as<args::ParseError>([&]{
        args::Flag flag_d(group2, "d", "test flag", {'b', "flagd"});
    });
}

void testDuplicateLongInTwoGroups()
{
    args::ArgumentParser parser("This is a test program.", "This goes after the options.");
    args::Group group1(parser, "This is a test group.", args::Group::Validators::DontCare);
    args::Flag flag_a(group1, "a", "test flag", {'a', "flaga"});
    args::Flag flag_b(group1, "b", "test flag", {'b', "flagb"});
    args::Group group2(parser, "This is a test group.", args::Group::Validators::DontCare);
    args::Flag flag_c(group1, "c", "test flag", {'c', "flagc"});
    test::require_throws_as<args::ParseError>([&]{
        args::Flag flag_d(group1, "d", "test flag", {'d', "flagb"});
    });
}

void testFlagReuseAcrossCommandBoundary()
{
    // A flag inside a command and a flag in an ancestor scope may share a
    // name: they live in separate namespaces. Declaring the command flag
    // first must not be reported as a duplicate.
    args::ArgumentParser parser("This is a test program.", "This goes after the options.");
    args::Group commands(parser, "commands");
    args::Command run(commands, "run", "run command");
    args::Flag run_message(run, "message", "message", {'m', "message"});
    test::require_nothrow([&]{
        args::Flag top_message(parser, "message", "top message", {'m', "message"});
    });
}

void testFlagReuseInNestedGroupAcrossCommandBoundary()
{
    args::ArgumentParser parser("This is a test program.", "This goes after the options.");
    args::Group commands(parser, "commands");
    args::Command run(commands, "run", "run command");
    args::Group runopts(run, "run options");
    args::Flag run_message(runopts, "message", "message", {'m', "message"});
    test::require_nothrow([&]{
        args::Flag top_message(parser, "message", "top message", {'m', "message"});
    });
}

void testFlagReuseBetweenSiblingCommands()
{
    args::ArgumentParser parser("This is a test program.", "This goes after the options.");
    args::Group commands(parser, "commands");
    args::Command commit(commands, "commit", "commit command");
    args::Flag commit_message(commit, "message", "message", {'m', "message"});
    args::Command tag(commands, "tag", "tag command");
    test::require_nothrow([&]{
        args::Flag tag_message(tag, "message", "message", {'m', "message"});
    });
}

void testDuplicateWithinCommandStillDetected()
{
    args::ArgumentParser parser("This is a test program.", "This goes after the options.");
    args::Group commands(parser, "commands");
    args::Command run(commands, "run", "run command");
    args::Flag run_aone(run, "aone", "test flag", {'a', "aone"});
    test::require_throws_as<args::ParseError>([&]{
        args::Flag run_atwo(run, "atwo", "test flag", {'a', "atwo"});
    });
}

int main()
{
    testDuplicateShort();
    testDuplicateLong();
    testDuplicateShortInGroup();
    testDuplicateLongInGroup();
    testDuplicateShortInTwoGroups();
    testDuplicateLongInTwoGroups();
    testFlagReuseAcrossCommandBoundary();
    testFlagReuseInNestedGroupAcrossCommandBoundary();
    testFlagReuseBetweenSiblingCommands();
    testDuplicateWithinCommandStillDetected();

}