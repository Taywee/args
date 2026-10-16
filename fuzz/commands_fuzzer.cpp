/*
 * Fuzzer for the command/subparser lifecycle of args.hxx: static and
 * nested commands, callback-created arguments, global options, required
 * commands, kick-out remainders, and repeated parsing.
 *
 * The command and subparser regions of the library are essentially unhit
 * by the hosted parse_args_fuzzer, which registers no commands at all.
 *
 * Following the contract documented by test/subparser_commands.cxx:
 * registered objects remain alive for the parser's lifetime, and command
 * callbacks call Parse() on their Subparser before consuming results.
 * The harness parses twice per input to exercise Reset() and command
 * re-selection between rounds.
 */

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include <fuzzer/FuzzedDataProvider.h>

#include <args.hxx>

namespace
{

const char *const kTokens[] = {
    "add", "commit", "remote", "--git-dir", ".git", "--help", "-h",
    "a.txt", "b.txt", "-a", "--amend", "-m", "msg one", "--message=x",
    "origin", "https://example.com/src.git",
    "--", "later", "--unknown", "-x", "",
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

    // Global options shared by every subcommand (test/global_options.cxx).
    args::Group globals;
    args::ValueFlag<std::string> gitdir(globals, "path", "", {"git-dir"});
    args::HelpFlag help(globals, "help", "help", {"help"});

    args::ArgumentParser parser("commands_fuzzer");
    args::GlobalOptions globalOptions(parser, globals);
    args::PositionalList<std::string> paths(parser, "paths", "files", args::Options::Global);

    std::vector<std::string> addPaths;
    std::vector<std::string> commitInfo;
    std::vector<std::string> nestedValues;

    args::Command add(parser, "add", "Add file contents to the index", [&](args::Subparser &sp)
    {
        args::PositionalList<std::string> list(sp, "paths", "files to add");
        sp.Parse();
        addPaths.assign(std::begin(list), std::end(list));
    });

    args::Command commit(parser, "commit", "Record changes", [&](args::Subparser &sp)
    {
        args::Flag amend(sp, "amend", "amend", {'a', "amend"});
        args::ValueFlag<std::string> message(sp, "message", "message", {'m', "message"});
        amend.KickOut(true);
        sp.Parse();
        if (amend)
        {
            (void)amend.Get();
        }
        if (message)
        {
            commitInfo.emplace_back(*message);
        }
    });

    // Nested command: a command created inside another command's subparser.
    args::Command remote(parser, "remote", "Manage sets of tracked repositories", [&](args::Subparser &sp)
    {
        args::Command addRemote(sp, "add", "Add a tracked repository", [&](args::Subparser &sp2)
        {
            args::Positional<std::string> name(sp2, "name", "remote name");
            args::Positional<std::string> url(sp2, "url", "remote url");
            sp2.Parse();
            if (name)
            {
                nestedValues.emplace_back(*name);
            }
            if (url)
            {
                nestedValues.emplace_back(*url);
            }
        });
        sp.Parse();
    });

    parser.RequireCommand(fdp.ConsumeBool());

    for (int round = 0; round < 2; ++round)
    {
        const size_t tokenCount = fdp.ConsumeIntegralInRange<size_t>(1, 10);
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

        // Consume results so command state and getters are exercised.
        if (add)
        {
            (void)addPaths.size();
        }
        if (commit)
        {
            (void)commitInfo.size();
        }
        if (remote)
        {
            (void)nestedValues.size();
        }
        if (gitdir)
        {
            (void)*gitdir;
        }
        if (paths)
        {
            (void)paths->size();
        }

        addPaths.clear();
        commitInfo.clear();
        nestedValues.clear();
    }

    return 0;
}
