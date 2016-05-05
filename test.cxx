/* Copyright © 2016 Taylor C. Richberger <taywee@gmx.com>
 * This code is released under the license described in the LICENSE file
 */

#include <iostream>
#include <list>
#include <clocale>

#include <args.hxx>

int main(int argc, char **argv)
{
    setlocale(LC_ALL, "");
    args::ArgumentParser parser("This is a test program.  It has a lot of really cool things about it, and it also has a really really long description line that is going to necessitate some line breaks.", "As you saw above, this previously mentioned program has a lot of long things, and therefore necessitates some line breaking");
    args::Group mutexgroup(parser, "Group test", args::Group::Validators::AllOrNone);
    args::Flag aflag(mutexgroup, "a", "This is flag a", args::Matcher({'a'}));
    args::Flag bflag(mutexgroup, "b", "This is flag b", args::Matcher({'b'}));
    args::Flag cflag(mutexgroup, "c", "This is flag c", args::Matcher({'c'}));
    args::HelpFlag help(parser, "help", "Display this help menu", args::Matcher({'h'}, {"help"}));
    args::Flag fooflag(parser, "foo", "This is a foo flag", args::Matcher({'f', 'F'}, {"Foo", "foo"}));
    args::ArgFlag<std::string> bararg(parser, "bar", "This is a bar flag", args::Matcher({'b', 'B'}, {"Bar", "bar"}));
    args::ArgFlag<double> baz(parser, "baz", "This is a baz flag, and it is long enough that it needs to be wrapped.", args::Matcher({'z', 'Z'}, {"Baz", "baz"}));
    args::ArgFlagList<double> list(parser, "flaglist", "This is a list flag", args::Matcher({'l'}, {"list"}));
    args::PosArg<std::string> pos(parser, "Position", "This is a position arg");
    args::PosArgList<double> poslist(parser, "Position list", "This is a position list arg");
    args::PosArg<double> pos2(parser, "Position", "This is a position arg which has a long enough description to probably necessitate wrapping");
    args::PosArg<double> pos3(parser, "Position", "This is a position arg");
    args::PosArg<double> pos4(parser, "Pösitiön", "This is a position arg");
    args::PosArg<double> pos5(parser, "Position", "This is a position arg");
    args::PosArg<double> pos6(parser, "Position", "This is a position arg");
    args::PosArgList<double> poslist1(parser, "Position list", "This is a position list arg");
    args::PosArgList<double> poslist2(parser, "Position list", "This is a position list arg");
    args::PosArgList<double> poslist3(parser, "Position list", "This is a position list arg");
    args::PosArg<double> pos7(parser, "Position", "This is a position arg");
    args::PosArg<double> pos8(parser, "Position", "This is a position arg");
    args::PosArgList<double> poslist4(parser, "Position list", "This is a position list arg");
    args::PosArg<double> pos9(parser, "Position", "This is a position arg");
    args::Counter counter(parser, "counter", "This is a counter flag", args::Matcher({'c'}));
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
    if (aflag)
    {
        std::cout << "A flag found" << std::endl;
    }
    if (bflag)
    {
        std::cout << "B flag found" << std::endl;
    }
    if (cflag)
    {
        std::cout << "C flag found" << std::endl;
    }
    if (fooflag)
    {
        std::cout << "Foo flag found" << std::endl;
    }
    if (bararg)
    {
        std::cout << "Bar arg found: " << bararg.value << std::endl;
    }
    if (baz)
    {
        std::cout << "Baz arg found: " << baz.value << std::endl;
    }
    if (counter)
    {
        std::cout << "counter found: " << counter.count << std::endl;
    }
    if (list)
    {
        std::cout << "list found: " << std::endl;
        for (const auto &item: list.values)
        {
        std::cout << "- " << item << std::endl;
        }
    }
    if (pos)
    {
        std::cout << "pos found: " << pos.value << std::endl;
    }
    if (poslist)
    {
        std::cout << "poslist found: " << std::endl;
        for (const auto &item: poslist.values)
        {
        std::cout << "- " << item << std::endl;
        }
    }

    return 0;
}
