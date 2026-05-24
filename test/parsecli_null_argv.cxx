#include "test_helpers.hxx"
#include "args.hxx"

int main()
{
    args::ArgumentParser parser("parser");

    const char *argv[] = {"prog", "--flag", nullptr};

    test::require_throws_as<args::ParseError>([&]
    {
        parser.ParseCLI(3, argv);
    });

    return 0;
}
