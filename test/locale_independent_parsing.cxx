#include "test_common.hxx"
#include <args.hxx>
#include "test_helpers.hxx"

#include <locale>

// Custom numpunct facet that uses ',' as the decimal point and ';' as
// the thousands separator. Installing this in the global C++ locale
// reproduces what happens on systems where std::locale::global has been
// set to a locale with non-"C" numeric formatting (e.g. de_DE), without
// requiring any specific system locale to be installed on the test host.
struct CommaDecimalPunct : std::numpunct<char>
{
protected:
    char do_decimal_point() const override { return ','; }
    char do_thousands_sep() const override { return ';'; }
    std::string do_grouping() const override { return "\3"; }
};

int main()
{
    // Save the original global locale so the test does not leak state.
    const std::locale original_global = std::locale();

    // Install an anonymous global locale derived from the classic locale,
    // overriding numpunct only. This anonymous-locale form does not call
    // setlocale() under the hood, so it cannot perturb anything outside
    // the C++ locale machinery.
    std::locale::global(std::locale(std::locale::classic(), new CommaDecimalPunct));

    // Regardless of the global locale's decimal point, command-line
    // numeric arguments must keep using '.' as the decimal separator.
    // Before the fix, this would parse "3.14" as 3 (with ".14" trailing)
    // and either silently truncate or be rejected.
    {
        args::ArgumentParser parser("locale-independent parsing test");
        args::ValueFlag<double> rate(parser, "RATE", "rate", {'r', "rate"});

        std::vector<std::string> a{"--rate", "3.14"};
        parser.ParseArgs(a);
        const double v = args::get(rate);
        test::require(v > 3.139 && v < 3.141);
    }

    // The '.'-as-decimal contract also holds for float.
    {
        args::ArgumentParser parser("locale-independent parsing test");
        args::ValueFlag<float> rate(parser, "RATE", "rate", {'r', "rate"});

        std::vector<std::string> a{"--rate", "2.5"};
        parser.ParseArgs(a);
        const float v = args::get(rate);
        test::require(v > 2.49f && v < 2.51f);
    }

    // The ',' character is not a decimal separator on the command line,
    // so it must be rejected as trailing garbage regardless of locale.
    test::require_throws_as<args::ParseError>([] {
        args::ArgumentParser parser("locale-independent parsing test");
        args::ValueFlag<double> rate(parser, "RATE", "rate", {'r', "rate"});
        std::vector<std::string> a{"--rate", "3,14"};
        parser.ParseArgs(a);
    });

    // Restore the original global locale so we do not perturb later
    // tests sharing the process (defensive; tests are independent
    // executables under CTest but this is cheap and safe).
    std::locale::global(original_global);

    return 0;
}
