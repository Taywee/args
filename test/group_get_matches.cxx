/* Copyright (c) Taylor Richberger <taylor@axfive.net>
 * This code is released under the license described in the LICENSE file
 */

#include "test_common.hxx"

#include <args.hxx>

#include "test_helpers.hxx"

inline void require_contains(const std::vector<args::Base *> vec, const args::Flag *target)
{
    test::require(std::find(vec.begin(), vec.end(), target) != vec.end());
}

int main()
{
    args::ArgumentParser parser("This is a test program.", "This goes after the options.");
    args::Group nonegroup(parser, "this group tests when no flags match.", args::Group::Validators::DontCare);
    args::Flag none_a(nonegroup, "a", "test flag", {'a'});
    args::Flag none_b(nonegroup, "b", "test flag", {'b'});
    args::Flag none_c(nonegroup, "c", "test flag", {'c'});
    args::Group onegroup(parser, "this group tests when one flag matches.", args::Group::Validators::DontCare);
    args::Flag one_d(onegroup, "d", "test flag", {'d'});
    args::Flag one_e(onegroup, "e", "test flag", {'e'});
    args::Flag one_f(onegroup, "f", "test flag", {'f'});
    args::Group allgroup(parser, "this group tests when all flags match.", args::Group::Validators::DontCare);
    args::Flag all_g(allgroup, "a", "test flag", {'g'});
    args::Flag all_h(allgroup, "b", "test flag", {'h'});
    args::Flag all_i(allgroup, "c", "test flag", {'i'});

    parser.ParseArgs(std::vector<std::string>{});
    auto none_matched = nonegroup.GetMatchedChildren();
    test::require(none_matched.empty());
    parser.Reset();
    
    parser.ParseArgs(std::vector<std::string>{"-e"});
    auto onematched = onegroup.GetMatchedChildren();
    test::require(onematched.size() == 1);
    test::require(onematched[0] == &one_e);
    parser.Reset();
    
    parser.ParseArgs(std::vector<std::string>{"-g", "-h", "-i"});
    auto allmatched = allgroup.GetMatchedChildren();
    test::require(allmatched.size() == 3);
    require_contains(allmatched, &all_g);
    require_contains(allmatched, &all_h);
    require_contains(allmatched, &all_i);
}