/* Copyright © 2016 Taylor C. Richberger <taywee@gmx.com>
 * This code is released under the license described in the LICENSE file
 */

#include <iostream>

#include <args.hxx>

int main(int argc, char **argv)
{
    args::ArgumentParser parser("This command likes to break your disks");
    parser.LongPrefix("/");
    parser.LongSeparator(":");
    args::HelpFlag help(parser, "HELP", "Show this help menu.", args::Matcher({"help"}));
    args::ArgFlag<long> bs(parser, "BYTES", "Block size", args::Matcher({"bs"}), 512);
    args::ArgFlag<long> skip(parser, "BYTES", "Bytes to skip", args::Matcher({"skip"}), 0);
    args::ArgFlag<std::string> input(parser, "BLOCK SIZE", "Block size", args::Matcher({"if"}));
    args::ArgFlag<std::string> output(parser, "BLOCK SIZE", "Block size", args::Matcher({"of"}));
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
    std::cout << "bs = " << bs.value << std::endl;
    std::cout << "skip = " << skip.value << std::endl;
    if (input) { std::cout << "if = " << input.value << std::endl; }
    if (output) { std::cout << "of = " << output.value << std::endl; }
    return 0;
}
