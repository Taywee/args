/* Copyright © 2016 Taylor C. Richberger <taywee@gmx.com>
 * This code is released under the license described in the LICENSE file
 */

#include <string>
#include <vector>
#include <list>
#include <functional>

namespace args
{
    class Base
    {
        protected:
            bool matched;

        public:
            Base() : matched(false) {}
            virtual ~Base() {}

            virtual bool Matched() const
            {
                return matched;
            }
    };

    class Group : public Base
    {
        private:
            std::vector<Base*> children;
            std::function<bool(int, int)> validator;

        public:

            Group(const std::function<bool(int, int)> &validator = Validators::DontCare) : validator(validator) {}
            virtual ~Group() {}

            void Add(Base &child)
            {
                children.emplace_back(&child);
            }

            int MatchedChildren() const
            {
                int sum = 0;
                for (const Base * child: children)
                {
                    if (child->Matched())
                    {
                        ++sum;
                    }
                }
                return sum;
            }

            virtual bool Matched() const override
            {
                return validator(children.size(), MatchedChildren());
            }

            struct Validators
            {
                static bool Xor(int children, int matched)
                {
                    return matched == 1;
                }

                static bool AtLeastOne(int children, int matched)
                {
                    return matched >= 1;
                }

                static bool AtMostOne(int children, int matched)
                {
                    return matched <= 1;
                }

                static bool All(int children, int matched)
                {
                    return children == matched;
                }

                static bool DontCare(int children, int matched)
                {
                    return true;
                }

                static bool CareTooMuch(int children, int matched)
                {
                    return false;
                }

                static bool None(int children, int matched)
                {
                    return matched == 0;
                }
            };
    };

    class ArgumentParser
    {
        private:
            std::string prog;
            std::string description;
            std::string epilog;

            Group args;

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
