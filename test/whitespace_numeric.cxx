#include "test_common.hxx"
#include <args.hxx>
#include "test_helpers.hxx"

int main()
{
    args::ArgumentParser parser("This is a test program.", "This goes after the options.");
    args::ValueFlag<int> foo(parser, "FOO", "test flag", {'f', "foo"});
    args::ValueFlag<unsigned int> uid(parser, "UID", "numeric id", {'u', "uid"});

    // Valid inputs
    {
        std::vector<std::string> args{"--foo", "123"};
        parser.ParseArgs(args);
        test::require(args::get(foo) == 123);
    }
    
    {
        std::vector<std::string> args{"--foo", " 123"};
        parser.ParseArgs(args);
        test::require(args::get(foo) == 123);
    }
    
    {
        std::vector<std::string> args{"--foo", "123 "};
        parser.ParseArgs(args);
        test::require(args::get(foo) == 123);
    }
    
    {
        std::vector<std::string> args{"--foo", "+123"};
        parser.ParseArgs(args);
        test::require(args::get(foo) == 123);
    }

    {
        std::vector<std::string> args{"--uid", "0x10"};
        parser.ParseArgs(args);
        test::require(args::get(uid) == 16);
    }

    {
        std::vector<std::string> args{"--uid", "010"};
        parser.ParseArgs(args);
        test::require(args::get(uid) == 8);
    }

    // Failing inputs
    test::require_throws_as<args::ParseError>([&] { 
        std::vector<std::string> args{"--foo", "   "};
        parser.ParseArgs(args); 
    });
    test::require_throws_as<args::ParseError>([&] { 
        std::vector<std::string> args{"--foo", ""};
        parser.ParseArgs(args); 
    });
    test::require_throws_as<args::ParseError>([&] { 
        std::vector<std::string> args{"--uid", "   "};
        parser.ParseArgs(args); 
    });
    
    return 0;
}
