/*
 * Fuzzer for the help rendering and text formatting machinery of args.hxx.
 *
 * The original OSS-Fuzz target (parse_args_fuzzer) only parses arguments
 * and never renders help, so description building, word wrapping, prog
 * lines, gutters, and choices/defaults rendering stayed untested.
 *
 * This harness:
 *   - builds a schema with fuzzer-varied descriptions, defaults, choices,
 *     hidden options, a validated group, and positionals,
 *   - varies every ArgumentParser helpParams field, including pathological
 *     narrow widths that historically caused unsigned underflow
 *     (see test/help_underflow.cxx),
 *   - parses a mix of recognized and arbitrary tokens so help rendering
 *     runs against varied matched state, and
 *   - renders help through all available paths: ArgumentParser::Help(),
 *     operator<<(std::ostream&, const ArgumentParser&),
 *     GetDescription()/GetProgramLine(), and the HelpFlag exception path.
 */

#include <cstddef>
#include <cstdint>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

#include <fuzzer/FuzzedDataProvider.h>

#include <args.hxx>

namespace
{

// Prevent the rendered strings from being dead-code eliminated.
unsigned long g_sink = 0;

// Only combinations the setters accept: they throw on invalid input, and
// schema setup exceptions would test the validators, not the formatting
// code this harness targets.
const char *const kLongPrefixes[] = {"--", "/", "-", "---"};
const char *const kShortPrefixes[] = {"-", "/", ""};
const char *const kTerminators[] = {"--", "-", ""};
const char *const kLongSeparators[] = {"=", ":", "-", " "};

bool (*const kGroupValidators[])(const args::Group &) = {
    &args::Group::Validators::DontCare,
    &args::Group::Validators::AtMostOne,
    &args::Group::Validators::AtLeastOne,
    &args::Group::Validators::Xor,
    &args::Group::Validators::AllOrNone,
};

const char *const kTokens[] = {
    "-h", "--help", "-a", "--alpha", "-b", "--beta", "-n", "7", "--number=42",
    "--number=-3", "--number=x", "--number=", "-w", "one", "--words=two",
    "-c", "red", "--colour=nope", "-g", "2.5", "--gamma=1e3", "-d", "--",
    "positional", "-z", "--unknown", "",
};

template <typename T, size_t N>
constexpr size_t ArraySize(T (&)[N])
{
    return N;
}

} // namespace

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    FuzzedDataProvider fdp(data, size);

    // -- Schema, with fuzzer-varied text --------------------------------------
    args::ArgumentParser parser(fdp.ConsumeRandomLengthString(64),
                                fdp.ConsumeRandomLengthString(64));

    try
    {
        parser.LongPrefix(kLongPrefixes[fdp.ConsumeIntegralInRange<size_t>(0, ArraySize(kLongPrefixes) - 1)]);
        parser.ShortPrefix(kShortPrefixes[fdp.ConsumeIntegralInRange<size_t>(0, ArraySize(kShortPrefixes) - 1)]);
        parser.LongSeparator(kLongSeparators[fdp.ConsumeIntegralInRange<size_t>(0, ArraySize(kLongSeparators) - 1)]);
        parser.Terminator(kTerminators[fdp.ConsumeIntegralInRange<size_t>(0, ArraySize(kTerminators) - 1)]);
    }
    catch (...)
    {
    }

    args::HelpFlag help(parser, "help", "Displays this help menu", {'h', "help"});

    args::Options flagOptions = args::Options::None;
    switch (fdp.ConsumeIntegralInRange<unsigned>(0, 3))
    {
        case 1:
            flagOptions = args::Options::HiddenFromDescription;
            break;
        case 2:
            flagOptions = args::Options::HiddenFromUsage;
            break;
        case 3:
            flagOptions = args::Options::Hidden;
            break;
        default:
            break;
    }

    args::Flag alpha(parser, "alpha", fdp.ConsumeRandomLengthString(64), {'a', "alpha"}, flagOptions);
    args::CounterFlag beta(parser, "beta", fdp.ConsumeRandomLengthString(64), {'b', "beta"}, 0, flagOptions);
    args::ValueFlag<int> number(parser, "number", fdp.ConsumeRandomLengthString(64), {'n', "number"}, fdp.ConsumeIntegral<int>());
    args::ValueFlag<std::string> text(parser, "text", fdp.ConsumeRandomLengthString(64), {"text"}, fdp.ConsumeRandomLengthString(24));
    args::ValueFlagList<std::string> words(parser, "words", fdp.ConsumeRandomLengthString(64), {'w', "words"});

    // Must be std::unordered_map: MapFlag's Map template parameter defaults
    // to args::detail::unordered_map and std::map does not convert.
    const std::unordered_map<std::string, int> choices{{"red", 1}, {"green", 2}, {"blue", 3}};
    args::MapFlag<std::string, int> colour(parser, "colour", fdp.ConsumeRandomLengthString(64), {'c', "colour"}, choices);

    args::Group group(parser, fdp.ConsumeRandomLengthString(32),
                      kGroupValidators[fdp.ConsumeIntegralInRange<size_t>(0, ArraySize(kGroupValidators) - 1)]);
    args::ValueFlag<double> gamma(group, "gamma", fdp.ConsumeRandomLengthString(64), {'g', "gamma"}, 0.5);
    args::Flag delta(group, "delta", fdp.ConsumeRandomLengthString(64), {'d'});

    args::Positional<std::string> source(parser, "source", fdp.ConsumeRandomLengthString(64));
    args::PositionalList<std::string> extra(parser, "extra", fdp.ConsumeRandomLengthString(64));

    // -- Every HelpParams field, including pathological narrow widths ---------
    parser.helpParams.width = fdp.ConsumeIntegralInRange<unsigned>(0, 300);
    parser.helpParams.progindent = fdp.ConsumeIntegralInRange<unsigned>(0, 16);
    parser.helpParams.progtailindent = fdp.ConsumeIntegralInRange<unsigned>(0, 16);
    parser.helpParams.descriptionindent = fdp.ConsumeIntegralInRange<unsigned>(0, 16);
    parser.helpParams.flagindent = fdp.ConsumeIntegralInRange<unsigned>(0, 24);
    parser.helpParams.helpindent = fdp.ConsumeIntegralInRange<unsigned>(0, 96);
    parser.helpParams.eachgroupindent = fdp.ConsumeIntegralInRange<unsigned>(0, 8);
    parser.helpParams.gutter = fdp.ConsumeIntegralInRange<unsigned>(0, 8);
    parser.helpParams.showTerminator = fdp.ConsumeBool();
    parser.helpParams.showProglineOptions = fdp.ConsumeBool();
    parser.helpParams.showProglinePositionals = fdp.ConsumeBool();
    parser.helpParams.showCommandChildren = fdp.ConsumeBool();
    parser.helpParams.showCommandFullHelp = fdp.ConsumeBool();
    parser.helpParams.proglineShowFlags = fdp.ConsumeBool();
    parser.helpParams.proglinePreferShortFlags = fdp.ConsumeBool();
    parser.helpParams.useValueNameOnce = fdp.ConsumeBool();
    parser.helpParams.showValueName = fdp.ConsumeBool();
    parser.helpParams.addNewlineBeforeDescription = fdp.ConsumeBool();
    parser.helpParams.addChoices = fdp.ConsumeBool();
    parser.helpParams.addDefault = fdp.ConsumeBool();
    parser.helpParams.programName = fdp.ConsumeRandomLengthString(24);
    parser.helpParams.usageString = fdp.ConsumeRandomLengthString(24);
    parser.helpParams.optionsString = fdp.ConsumeRandomLengthString(24);
    parser.helpParams.proglineOptions = fdp.ConsumeRandomLengthString(16);
    parser.helpParams.proglineCommand = fdp.ConsumeRandomLengthString(16);
    parser.helpParams.valueOpen = fdp.ConsumeRandomLengthString(8);
    parser.helpParams.valueClose = fdp.ConsumeRandomLengthString(8);
    parser.helpParams.choiceString = fdp.ConsumeRandomLengthString(16);
    parser.helpParams.defaultString = fdp.ConsumeRandomLengthString(16);

    // -- Parse a token mix so rendering runs against varied matched state -----
    const size_t tokenCount = fdp.ConsumeIntegralInRange<size_t>(0, 8);
    std::vector<std::string> argv;
    argv.reserve(tokenCount);
    for (size_t i = 0; i < tokenCount; ++i)
    {
        if (fdp.ConsumeBool())
        {
            argv.emplace_back(kTokens[fdp.ConsumeIntegralInRange<size_t>(0, ArraySize(kTokens) - 1)]);
        }
        else
        {
            argv.emplace_back(fdp.ConsumeRandomLengthString(16));
        }
    }

    try
    {
        parser.ParseArgs(argv);
    }
    catch (...)
    {
    }

    // -- Render through every help path ----------------------------------------
    g_sink += parser.Help().size();

    std::ostringstream stream;
    stream << parser;
    g_sink += stream.str().size();

    g_sink += parser.GetDescription(parser.helpParams, 0).size();
    g_sink += parser.GetProgramLine(parser.helpParams).size();

    // Re-render with flipped boolean toggles to hit the alternate branches.
    parser.helpParams.addChoices = !parser.helpParams.addChoices;
    parser.helpParams.addDefault = !parser.helpParams.addDefault;
    parser.helpParams.addNewlineBeforeDescription = !parser.helpParams.addNewlineBeforeDescription;
    g_sink += parser.Help().size();

    return 0;
}
