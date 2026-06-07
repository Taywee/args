#include "args.hxx"
#include <thread>
#include <vector>
#include <iostream>
#include <mutex>

int main()
{
    args::ArgumentParser parser("concurrency test");
    args::ValueFlag<std::string> f(parser, "name", "desc", {'f', "foo"}, "default");
    args::Positional<std::string> pos(parser, "pos", "positional");

    const int threads = 8;
    const int iterations = 500;
    std::vector<std::thread> ths;
    std::atomic<int> failures{0};

    std::mutex parseMutex;
    for (int t = 0; t < threads; ++t)
    {
        ths.emplace_back([&parser, &failures, &parseMutex, t, iterations]() {
            try
            {
                for (int i = 0; i < iterations; ++i)
                {
                    std::vector<std::string> args = {"--foo=val", "posval"};
                    // Alternate different arguments to exercise parsing paths
                    if ((i + t) % 2 == 0)
                        args[0] = "--foo=thread" + std::to_string(t);

                    std::lock_guard<std::mutex> lock(parseMutex);
                    parser.ParseArgs(args);
                }
            }
            catch (const std::exception &e)
            {
                ++failures;
                std::cerr << "Thread " << t << " exception: " << e.what() << std::endl;
            }
            catch (...)
            {
                ++failures;
            }
        });
    }

    for (auto &th : ths) th.join();

    if (failures.load() != 0)
    {
        std::cerr << "Failures: " << failures.load() << std::endl;
        return 1;
    }

    std::cout << "Completed concurrent parsing without exception." << std::endl;
    return 0;
}
