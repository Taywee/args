/* Copyright © 2016 Taylor C. Richberger <taywee@gmx.com>
 * This code is released under the license described in the LICENSE file
 */

#include <algorithm>
#include <functional>
#include <list>
#include <sstream>
#include <string>
#include <vector>
#include <exception>

namespace args
{
    class ParseError : public std::runtime_error
    {
        public:
            ParseError(const char *problem) : std::runtime_error(problem) {}
            virtual ~ParseError() {};
    };

    // A class of "matchers", specifying short and long options that can possibly be matched
    class Matcher
    {
        private:
            std::vector<char> shortOpts;
            std::vector<std::string> longOpts;

        public:
            // Specify short and long opts separately as iterators
            template <typename ShortIt, typename LongIt>
            Matcher(ShortIt shortOptsStart, ShortIt shortOptsEnd, LongIt longOptsStart, LongIt longOptsEnd) :
                shortOpts(shortOptsStart, shortOptsEnd),
                longOpts(longOptsStart, longOptsEnd)
            {}

            // Specify short and long opts separately as iterables
            template <typename Short, typename Long>
            Matcher(Short &&shortIn, Long &&longIn) :
                Matcher(std::begin(shortIn), std::end(shortIn), std::begin(longIn), std::end(longIn))
            {}

            bool Match(const char opt) const
            {
                return std::find(shortOpts.begin(), shortOpts.end(), opt) != shortOpts.end();
            }

            bool Match(const std::string &opt) const
            {
                return std::find(longOpts.begin(), longOpts.end(), opt) != longOpts.end();
            }
    };

    // Base class for groups and individual argument types
    class Base
    {
        protected:
            bool matched;

        public:
            Base() : matched(false) {}
            virtual ~Base() {}

            virtual Base *Match(const std::string &arg) = 0;
            virtual Base *Match(const char arg) = 0;

            virtual bool Matched() const
            {
                return matched;
            }

            operator bool() const
            {
                return matched;
            }
    };

    // Base class that takes arguments
    class ArgBase : public Base
    {
        public:
            virtual ~ArgBase() {}
    };

    class Group : public Base
    {
        private:
            std::vector<Base*> children;
            std::function<bool(int, int)> validator;

        public:

            Group(const std::function<bool(int, int)> &validator = Validators::DontCare) : validator(validator) {}
            virtual ~Group() {}

            virtual Base *Match(const std::string &arg) override
            {
                for (Base *child: children)
                {
                    Base *match = child->Match(arg);
                    if (match)
                    {
                        return match;
                    }
                }
                return nullptr;
            }

            virtual Base *Match(const char arg) override
            {
                for (Base *child: children)
                {
                    Base *match = child->Match(arg);
                    if (match)
                    {
                        return match;
                    }
                }
                return nullptr;
            }

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

            std::string longprefix;
            std::string shortprefix;

            Group group;

        public:
            ArgumentParser(
                const std::string &description,
                const std::string &longprefix = "--",
                const std::string &shortprefix = "-",
                const std::string &prog = std::string(),
                const std::string &epilog = std::string()
                ) :
                    prog(prog),
                    description(description),
                    epilog(epilog),
                    longprefix(longprefix),
                    shortprefix(shortprefix),
                    group(Group::Validators::DontCare) {}

            void Add(Base &item)
            {
                group.Add(item);
            }

            void ParseArgs(const std::vector<std::string> &args)
            {
                for (auto it = std::begin(args); it != std::end(args); ++it)
                {
                    const std::string &chunk = *it;
                    if (chunk.find(longprefix) == 0 && chunk.size() > longprefix.size())
                    {
                        const std::string argchunk(chunk.substr(longprefix.size()));
                        Base *base = group.Match(argchunk);
                        if (base)
                        {
                            // Do match logic here, specifically passing an
                            // argument, if necessary, to the base.  Query the
                            // base and see if it takes an argument
                        } else
                        {
                            std::ostringstream problem;
                            problem << "Argument could not be matched: " << chunk;
                            throw ParseError(problem.str().c_str());
                        }
                    } else if (chunk.find(shortprefix) == 0 && chunk.size() > shortprefix.size())
                    {
                        const std::string argchunk(chunk.substr(shortprefix.size()));
                        for (const char &arg: argchunk)
                        {
                            Base *base = group.Match(arg);
                            if (base)
                            {
                                // Do match logic here, specifically passing an
                                // argument, if necessary, to the base.  Query the
                                // base and see if it takes an argument
                            } else
                            {
                                std::ostringstream problem;
                                problem << "Argument could not be matched: " << arg;
                                throw ParseError(problem.str().c_str());
                            }
                        }
                    }
                }
            }

            void ParseCLI(const int argc, const char * const * const argv)
            {
                if (prog.empty())
                {
                    prog.assign(argv[0]);
                }
                std::vector<std::string> args;
                for (int i = 1; i < argc; ++i)
                {
                    args.emplace_back(argv[i]);
                }
                ParseArgs(args);
            }
    };

    // Boolean argument matcher
    class Flag : public Base
    {
        private:
            Matcher matcher;

        public:
            //template <typename ... Types>
            //Flag(std::string help, Types ...args): matcher(args...) {}

            template <typename ... Types>
            Flag(ArgumentParser &parser, std::string help, Types&& ...args): matcher(args...)
            {
                parser.Add(*this);
            }

            template <typename Short, typename Long>
            Flag(ArgumentParser &parser, std::string help, const std::initializer_list<Short> &shortIn, const std::initializer_list<Long> &longIn): matcher(shortIn, longIn)
            {
                parser.Add(*this);
            }

            virtual ~Flag() {}

            virtual Base *Match(const std::string &arg) override
            {
                if (matcher.Match(arg))
                {
                    matched = true;
                    return this;
                }
                return nullptr;
            }

            virtual Base *Match(const char arg) override
            {
                if (matcher.Match(arg))
                {
                    matched = true;
                    return this;
                }
                return nullptr;
            }
    };

}
