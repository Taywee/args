/* Copyright (c) Taylor Richberger <taylor@axfive.net>
 * This code is released under the license described in the LICENSE file
 *
 * Shared types and helpers used by the per-test executables.
 * Independent of args.hxx so it can be included before or after.
 */

#ifndef ARGS_TEST_COMMON_HXX
#define ARGS_TEST_COMMON_HXX

#include <algorithm>
#include <cctype>
#include <istream>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <vector>

inline std::istream& operator>>(std::istream& is, std::tuple<int, int>& ints)
{
    is >> std::get<0>(ints);
    is.get();
    is >> std::get<1>(ints);
    return is;
}

struct DoublesReader
{
    void operator()(const std::string &, const std::string &value, std::tuple<double, double> &destination)
    {
        size_t commapos = 0;
        std::get<0>(destination) = std::stod(value, &commapos);
        std::get<1>(destination) = std::stod(std::string(value, commapos + 1));
    }
};

enum class MappingEnum
{
    def,
    foo,
    bar,
    red,
    yellow,
    green
};

struct ToLowerReader
{
    void operator()(const std::string &, const std::string &value, std::string &destination)
    {
        destination = value;
        std::transform(destination.begin(), destination.end(), destination.begin(), [](char c) -> char { return static_cast<char>(std::tolower(c)); });
    }
};

struct StringAssignable
{
    StringAssignable() = default;
    StringAssignable(const std::string &p) : path(p) {}
    std::string path;

    friend std::istream &operator >> (std::istream &s, StringAssignable &a)
    { return s >> a.path; }
};

#endif
