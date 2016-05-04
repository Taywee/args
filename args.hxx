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
            const std::vector<char> shortOpts;
            const std::vector<std::string> longOpts;

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
                shortOpts(std::begin(shortIn), std::end(shortIn)), longOpts(std::begin(longIn), std::end(longIn))
            {}

            Matcher(const std::initializer_list<char> &shortIn, const std::initializer_list<std::string> &longIn) :
                shortOpts(std::begin(shortIn), std::end(shortIn)), longOpts(std::begin(longIn), std::end(longIn))
            {}

            Matcher(const std::initializer_list<char> &shortIn) :
                shortOpts(std::begin(shortIn), std::end(shortIn))
            {}

            Matcher(const std::initializer_list<std::string> &longIn) :
                longOpts(std::begin(longIn), std::end(longIn))
            {}

            Matcher(const Matcher &other) : shortOpts(other.shortOpts), longOpts(other.longOpts)
            {}

            Matcher(Matcher &&other) : shortOpts(std::move(other.shortOpts)), longOpts(std::move(other.longOpts))
            {}

            ~Matcher() {}

            bool Match(const char opt) const
            {
                return std::find(std::begin(shortOpts), std::end(shortOpts), opt) != shortOpts.end();
            }

            bool Match(const std::string &opt) const
            {
                return std::find(std::begin(longOpts), std::end(longOpts), opt) != longOpts.end();
            }
    };

    // Base class for groups and individual argument types
    class Base
    {
        protected:
            bool matched;
            const std::string help;

        public:
            Base(const std::string &help) : matched(false), help(help) {}
            virtual ~Base() {}

            virtual Base *Match(const std::string &arg) = 0;
            virtual Base *Match(const char arg) = 0;

            virtual bool Matched() const noexcept
            {
                return matched;
            }

            operator bool() const noexcept
            {
                return matched;
            }
    };

    // Base class that takes arguments
    class ArgBase : public Base
    {
        public:
            ArgBase(const std::string &help) : Base(help) {}
            virtual ~ArgBase() {}
            virtual void ParseArg(const std::string &arg, const std::string &value) = 0;
    };

    class Group : public Base
    {
        private:
            std::vector<Base*> children;
            std::function<bool(int, int)> validator;

        public:

            Group(const std::string &help, const std::function<bool(int, int)> &validator = Validators::DontCare) : Base(help), validator(validator) {}
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

            virtual bool Matched() const noexcept override
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

            std::string longseparator;

            Group group;

        public:
            operator Group&()
            {
                return group;
            }

            ArgumentParser(
                const std::string &description,
                const std::string &longprefix = "--",
                const std::string &shortprefix = "-",
                const std::string &longseparator = "=",
                const std::string &prog = std::string(),
                const std::string &epilog = std::string()
                ) :
                    prog(prog),
                    description(description),
                    epilog(epilog),
                    longprefix(longprefix),
                    shortprefix(shortprefix),
                    longseparator(longseparator),
                    group("arguments", Group::Validators::DontCare) {}

            void ParseArgs(const std::vector<std::string> &args)
            {
                // Check all arg chunks
                for (auto it = std::begin(args); it != std::end(args); ++it)
                {
                    const std::string &chunk = *it;

                    // If a long arg was found
                    if (chunk.find(longprefix) == 0 && chunk.size() > longprefix.size())
                    {
                        const std::string argchunk(chunk.substr(longprefix.size()));
                        // Try to separate it, in case of a separator:
                        const auto separator = argchunk.find(longseparator);
                        const std::string arg = (separator != argchunk.npos ?
                            std::string(argchunk, 0, separator)
                            : argchunk);

                        Base *base = group.Match(arg);
                        if (base)
                        {
                            ArgBase *argbase = dynamic_cast<ArgBase *>(base);
                            if (argbase)
                            {
                                if (separator != argchunk.npos)
                                {
                                    argbase->ParseArg(arg, argchunk.substr(separator + longseparator.size()));
                                } else
                                {
                                    ++it;
                                    if (it == std::end(args))
                                    {
                                        std::ostringstream problem;
                                        problem << "Argument " << arg << " requires an argument but received none";
                                        throw ParseError(problem.str().c_str());
                                    } else
                                    {
                                        argbase->ParseArg(arg, *it);
                                    }
                                }
                            } else if (separator != argchunk.npos)
                            {
                                std::ostringstream problem;
                                problem << "Passed an argument into a non-argument flag: " << chunk;
                                throw ParseError(problem.str().c_str());
                            }
                        } else
                        {
                            std::ostringstream problem;
                            problem << "Argument could not be matched: " << chunk;
                            throw ParseError(problem.str().c_str());
                        }
                    } else if (chunk.find(shortprefix) == 0 && chunk.size() > shortprefix.size())
                    {
                        std::string argchunk(chunk.substr(shortprefix.size()));
                        for (auto argit = std::begin(argchunk); argit != std::end(argchunk); ++argit)
                        {
                            const char arg = *argit;

                            Base *base = group.Match(arg);
                            if (base)
                            {
                                ArgBase *argbase = dynamic_cast<ArgBase *>(base);
                                if (argbase)
                                {
                                    argchunk.erase(std::begin(argchunk), ++argit);
                                    if (!argchunk.empty())
                                    {
                                        argbase->ParseArg(std::string(1, arg), argchunk);
                                    } else
                                    {
                                        ++it;
                                        if (it == std::end(args))
                                        {
                                            std::ostringstream problem;
                                            problem << "Argument " << arg << " requires an argument but received none";
                                            throw ParseError(problem.str().c_str());
                                        } else
                                        {
                                            argbase->ParseArg(std::string(1, arg), *it);
                                        }
                                    }
                                    // Because this argchunk is done regardless
                                    break;
                                }
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
            const Matcher matcher;

        public:
            Flag(Group &group, const std::string &help, const Matcher &matcher): Base(help), matcher(matcher)
            {
                group.Add(*this);
            }

            Flag(Group &group, const std::string &help, Matcher &&matcher): Base(help), matcher(std::move(matcher))
            {
                group.Add(*this);
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

    // Count matches
    class Counter : public Base
    {
        private:
            const Matcher matcher;
            unsigned int count;

        public:
            Counter(Group &group, const std::string &help, const Matcher &matcher, const unsigned int startcount = 0): Base(help), matcher(matcher), count(startcount)
            {
                group.Add(*this);
            }

            Counter(Group &group, const std::string &help, Matcher &&matcher, const unsigned int startcount = 0): Base(help), matcher(std::move(matcher)), count(startcount)
            {
                group.Add(*this);
            }

            virtual ~Counter() {}

            virtual Base *Match(const std::string &arg) override
            {
                if (matcher.Match(arg))
                {
                    matched = true;
                    ++count;
                    return this;
                }
                return nullptr;
            }

            virtual Base *Match(const char arg) override
            {
                if (matcher.Match(arg))
                {
                    matched = true;
                    ++count;
                    return this;
                }
                return nullptr;
            }

            unsigned int Count()
            {
                return count;
            }
    };

    template <typename T>
    void ArgReader(const std::string &arg, const std::string &value, T &destination)
    {
        std::istringstream ss(value);
        ss >> destination;

        if (ss.rdbuf()->in_avail() > 0)
        {
            std::ostringstream problem;
            problem << "Argument " << arg << " received invalid value type " << value;
            throw ParseError(problem.str().c_str());
        }
    }

    template <>
    void ArgReader<std::string>(const std::string &arg, const std::string &value, std::string &destination)
    {
        destination.assign(value);
    }

    template <typename T, void (*Reader)(const std::string &, const std::string &, T&) = ArgReader<T>>
    class ArgFlag : public ArgBase
    {
        private:
            const Matcher matcher;
            T value;

        public:
            ArgFlag(Group &group, const std::string &help, const Matcher &matcher): ArgBase(help), matcher(matcher)
            {
                group.Add(*this);
            }

            ArgFlag(Group &group, const std::string &help, Matcher &&matcher): ArgBase(help), matcher(std::move(matcher))
            {
                group.Add(*this);
            }

            virtual ~ArgFlag() {}

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

            virtual void ParseArg(const std::string &arg, const std::string &value) override
            {
                Reader(arg, value, this->value);
            }

            const T &Value()
            {
                return value;
            }
    };

    template <typename T, typename List = std::vector<T>, void (*Reader)(const std::string &, const std::string &, T&) = ArgReader<T>>
    class ArgFlagList : public ArgBase
    {
        private:
            const Matcher matcher;
            List values;

        public:
            ArgFlagList(Group &group, const std::string &help, const Matcher &matcher): ArgBase(help), matcher(matcher)
            {
                group.Add(*this);
            }

            ArgFlagList(Group &group, const std::string &help, Matcher &&matcher): ArgBase(help), matcher(std::move(matcher))
            {
                group.Add(*this);
            }

            virtual ~ArgFlagList() {}

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

            virtual void ParseArg(const std::string &arg, const std::string &value) override
            {
                values.emplace_back();
                Reader(arg, value, values.back());
            }

            const List &Values()
            {
                return values;
            }
    };
}
