/* Copyright © 2016 Taylor C. Richberger <taywee@gmx.com>
 * This code is released under the license described in the LICENSE file
 */

#include <string>
#include <vector>

namespace args
{
    class ArgumentParser
    {
        private:
            std::string prog;
            std::string description;
            std::string epilog;

        public:
            ArgumentParser(
                const std::string &prog,
                const std::string &description,
                const std::string &epilog = std::string()) : prog(prog), description(description), epilog(epilog) {}

            void ParseArgs(const std::vector<std::string> &args)
            {
            }

            void ParseCLI(const int argc, const char * const * const argv)
            {
                std::vector<std::string> args;
                for (int i = 1; i < argc; ++i)
                {
                    args.emplace_back(argv[i]);
                }
                ParseArgs(args);
            }
    };
}
