/* Copyright © 2016 Taylor C. Richberger <taywee@gmx.com>
 * This code is released under the license described in the LICENSE file
 */

#include <iostream>

#include <args.hxx>

int main(int argc, char **argv)
{
    args::ArgumentParser parser("This is a test program.", "This goes after the options.");
    args::HelpFlag help(parser, "help", "Display this help menu", args::Matcher({'h'}, {"help"}));
    args::ArgFlag<int> integer(parser, "integer", "The integer flag", args::Matcher({'i'}));
    args::ArgFlagList<char> characters(parser, "characters", "The character flag", args::Matcher({'c'}));
    args::PosArg<std::string> foo(parser, "foo", "The foo position");
    args::PosArgList<double> numbers(parser, "numbers", "The numbers position list");
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
    catch (args::ValidationError e)
    {
        std::cerr << e.what() << std::endl;
        std::cerr << parser.Help();
        return 1;
    }
    if (integer) { std::cout << "i: " << integer.value << std::endl; }
    if (characters) { for (const auto ch: characters.values) { std::cout << "c: " << ch << std::endl; } }
    if (foo) { std::cout << "f: " << foo.value << std::endl; }
    if (numbers) { for (const auto nm: numbers.values) { std::cout << "n: " << nm << std::endl; } }
    return 0;
}
