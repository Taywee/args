/* Copyright (c) Taylor Richberger <taylor@axfive.net>
 * This code is released under the license described in the LICENSE file
 */

#include "test_common.hxx"

#include <args.hxx>

#include "test_helpers.hxx"

enum class LogLevel {
    ERROR,
    WARNING
};

using LogFlag = args::ConstantFlag<LogLevel>;

struct Version {
    int high;
    int low;
};

int main()
{
    args::ArgumentParser parser("This is a test program.", "This goes after the options.");
    args::Group logLevels(parser, "This is a test group.", args::Group::Validators::AtMostOne);
    LogFlag error(logLevels, "error", "test constant flag", {'e', "error"}, LogLevel::ERROR);
    LogFlag warning(logLevels, "warning", "test constant flag", {'w', "warning"}, LogLevel::WARNING);
    
    parser.ParseArgs(std::vector<std::string>{});
    auto noMatches = logLevels.GetFilteredChildren<LogFlag>(true);
    test::require(noMatches.empty());

    parser.Reset();
    parser.ParseArgs(std::vector<std::string>{"-e"});
    auto shortErrorMatches = logLevels.GetFilteredChildren<LogFlag>(true);
    test::require(shortErrorMatches.size() == 1);
    test::require(shortErrorMatches[0]->Get() == LogLevel::ERROR);
    test::require(**(shortErrorMatches[0]) == LogLevel::ERROR);

    parser.Reset();
    parser.ParseArgs(std::vector<std::string>{"--error"});
    auto longErrorMatches = logLevels.GetFilteredChildren<LogFlag>(true);
    test::require(longErrorMatches.size() == 1);
    test::require(longErrorMatches[0]->Get() == LogLevel::ERROR);
    test::require(**(longErrorMatches[0]) == LogLevel::ERROR);

    parser.Reset();
    parser.ParseArgs(std::vector<std::string>{"-w"});
    auto shortWarningMatches = logLevels.GetFilteredChildren<LogFlag>(true);
    test::require(shortWarningMatches.size() == 1);
    test::require(shortWarningMatches[0]->Get() == LogLevel::WARNING);
    test::require(**(shortWarningMatches[0]) == LogLevel::WARNING);

    parser.Reset();
    parser.ParseArgs(std::vector<std::string>{"--warning"});
    auto longWarningMatches = logLevels.GetFilteredChildren<LogFlag>(true);
    test::require(longWarningMatches.size() == 1);
    test::require(longWarningMatches[0]->Get() == LogLevel::WARNING);
    test::require(**(longWarningMatches[0]) == LogLevel::WARNING);

    // Reach into a class-typed constant through operator->. This instantiates
    // ConstantFlag<T>::operator->, which the checks above never do because they
    // use a ConstantFlag pointer (plain pointer arrow) rather than the flag
    // object itself.
    args::ArgumentParser versionParser("This is a test program.");
    args::ConstantFlag<Version> stable(versionParser, "stable", "stable version", {'s', "stable"}, Version{2, 5});
    versionParser.ParseArgs(std::vector<std::string>{"--stable"});
    test::require(stable->high == 2);
    test::require(stable->low == 5);
    test::require((*stable).high == 2);

    const args::ConstantFlag<Version> &constStable = stable;
    test::require(constStable->low == 5);

    return 0;
}
