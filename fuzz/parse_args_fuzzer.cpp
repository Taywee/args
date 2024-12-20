#include <cstdint>
#include <vector>

#include <fuzzer/FuzzedDataProvider.h>
#include <args.hxx>

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    auto fdp = FuzzedDataProvider(data, size);

    auto parser = args::ArgumentParser("parse_args_fuzzer");

    auto help = args::HelpFlag(parser, "help", "Displays this help menu", {'h', "help"});
    auto test = args::Flag(parser, "test", "Test flag", {'t', "test"});
    auto param = args::Flag(parser, "param", "Param flag", {'p', "param"});

    auto count = fdp.ConsumeIntegralInRange(1, 10);
    auto arguments = std::vector<std::string>();
    arguments.reserve(count);
    for (int i = 0; i < count; ++i) {
        arguments.emplace_back(fdp.ConsumeRandomLengthString());
    }

    try {
        parser.ParseArgs(arguments.begin(), arguments.end());
    } catch (...) {
    }

    return 0;
}

