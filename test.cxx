/* Copyright © 2016 Taylor C. Richberger <taywee@gmx.com>
 * This code is released under the license described in the LICENSE file
 */

#include <iostream>
#include <tuple>

#include <args.hxx>

std::istream& operator>>(std::istream& is, std::tuple<int, int>& ints)
{
    is >> std::get<0>(ints);
    is.get();
    is >> std::get<1>(ints);
    return is;
}

void DoublesReader(const std::string &name, const std::string &value, std::tuple<double, double> &destination)
{
    size_t commapos = 0;
    std::get<0>(destination) = std::stod(value, &commapos);
    std::get<1>(destination) = std::stod(std::string(value, commapos + 1));
}

int main(int argc, char **argv)
{
    args::ArgumentParser parser("This is a test program.");
    args::PosArg<std::tuple<int, int>> ints(parser, "INTS", "This takes a pair of integers.");
    args::PosArg<std::tuple<double, double>, DoublesReader> doubles(parser, "DOUBLES", "This takes a pair of doubles.");
    try
    {
        parser.ParseCLI(argc, argv);
    }
    catch (args::Help)
    {
        std::cout << parser.Help();
        return 0;
    }
    catch (args::ParseError e)
    {
        std::cerr << e.what() << std::endl;
        std::cerr << parser.Help();
        return 1;
    }
    if (ints)
    {
        std::cout << "ints found: " << std::get<0>(ints.value) << " and " << std::get<1>(ints.value) << std::endl;
    }
    if (doubles)
    {
        std::cout << "doubles found: " << std::get<0>(doubles.value) << " and " << std::get<1>(doubles.value) << std::endl;
    }
    return 0;
}
