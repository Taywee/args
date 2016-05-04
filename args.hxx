/* Copyright © 2016 Taylor C. Richberger <taywee@gmx.com>
 * This code is released under the license described in the LICENSE file
 */

#include <algorithm>
#include <functional>
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

    class ValidationError : public std::runtime_error
    {
        public:
            ValidationError(const char *problem) : std::runtime_error(problem) {}
            virtual ~ValidationError() {};
    };

    class Help : public std::exception
    {
        private:
            const std::string flag;
        public:
            Help(const std::string &flag) : flag(flag) {}
            virtual ~Help() {};

            virtual const char *what() const noexcept override
            {
                return flag.c_str();
            }
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

            virtual bool Matched() const noexcept
            {
                return matched;
            }

            operator bool() const noexcept
            {
                return Matched();
            }
    };

    // Named arguments, not including groups
    class NamedBase : public Base
    {
        protected:
            const std::string name;

        public:
            NamedBase(const std::string &name, const std::string &help) : Base(help), name(name) {}
            virtual ~NamedBase() {}

            const std::string &Name();
    };

    // Base class for flag arguments
    class FlagBase : public NamedBase
    {
        protected:
            const Matcher matcher;

        public:
            FlagBase(const std::string &name, const std::string &help, const Matcher &matcher) : NamedBase(name, help), matcher(matcher) {}
            FlagBase(const std::string &name, const std::string &help, Matcher &&matcher) : NamedBase(name, help), matcher(std::move(matcher)) {}

            virtual ~FlagBase() {}

            virtual FlagBase *Match(const std::string &arg)
            {
                if (matcher.Match(arg))
                {
                    matched = true;
                    return this;
                }
                return nullptr;
            }

            virtual FlagBase *Match(const char arg)
            {
                if (matcher.Match(arg))
                {
                    matched = true;
                    return this;
                }
                return nullptr;
            }
    };

    // Base class that takes arguments
    class ArgFlagBase : public FlagBase
    {
        public:
            ArgFlagBase(const std::string &name, const std::string &help, const Matcher &matcher) : FlagBase(name, help, matcher) {}
            ArgFlagBase(const std::string &name, const std::string &help, Matcher &&matcher) : FlagBase(name, help, std::move(matcher)) {}
            virtual ~ArgFlagBase() {}
            virtual void ParseArg(const std::string &value) = 0;
    };

    // Base class for positional arguments
    class PosBase : public NamedBase
    {
        protected:
            bool ready;

        public:
            PosBase(const std::string &name, const std::string &help) : NamedBase(name, help), ready(true) {}
            virtual ~PosBase() {}

            bool Ready()
            {
                return ready;
            }

            virtual void ParseArg(const std::string &value) = 0;
    };

    class Group : public Base
    {
        private:
            std::vector<Base*> children;
            std::function<bool(const Group &)> validator;

        public:
            Group(const std::string &help, const std::function<bool(const Group &)> &validator = Validators::DontCare) : Base(help), validator(validator) {}
            virtual ~Group() {}

            FlagBase *Match(const std::string &arg)
            {
                for (Base *child: children)
                {
                    FlagBase *flag = dynamic_cast<FlagBase *>(child);
                    Group *group = dynamic_cast<Group *>(child);
                    if (flag)
                    {
                        FlagBase *match = flag->Match(arg);
                        if (match)
                        {
                            return match;
                        }
                    } else if (group)
                    {
                        FlagBase *match = group->Match(arg);
                        if (match)
                        {
                            return match;
                        }
                    }
                }
                return nullptr;
            }

            FlagBase *Match(const char arg)
            {
                for (Base *child: children)
                {
                    FlagBase *flag = dynamic_cast<FlagBase *>(child);
                    Group *group = dynamic_cast<Group *>(child);
                    if (flag)
                    {
                        FlagBase *match = flag->Match(arg);
                        if (match)
                        {
                            return match;
                        }
                    } else if (group)
                    {
                        FlagBase *match = group->Match(arg);
                        if (match)
                        {
                            return match;
                        }
                    }
                }
                return nullptr;
            }

            PosBase *GetNextPos()
            {
                for (Base *child: children)
                {
                    PosBase *next = dynamic_cast<PosBase *>(child);
                    Group *group = dynamic_cast<Group *>(child);
                    if (group)
                    {
                        next = group->GetNextPos();
                    }
                    if (next and next->Ready())
                    {
                        return next;
                    }
                }
                return nullptr;
            }

            void Add(Base &child)
            {
                children.emplace_back(&child);
            }

            const std::vector<Base *> Children() const
            {
                return children;
            }

            std::vector<Base *>::size_type MatchedChildren() const
            {
                std::vector<Base *>::size_type sum = 0;
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
                return validator(*this);
            }

            struct Validators
            {
                static bool Xor(const Group &group)
                {
                    return group.MatchedChildren() == 1;
                }

                static bool AtLeastOne(const Group &group)
                {
                    return group.MatchedChildren() >= 1;
                }

                static bool AtMostOne(const Group &group)
                {
                    return group.MatchedChildren() <= 1;
                }

                static bool All(const Group &group)
                {
                    return group.Children().size() == group.MatchedChildren();
                }

                static bool AllOrNone(const Group &group)
                {
                    return (All(group) || None(group));
                }

                static bool AllChildGroups(const Group &group)
                {
                    for (const auto child: group.Children())
                    {
                        const Group *group = dynamic_cast<Group *>(child);
                        if (group && (!group->Matched()))
                        {
                            return false;
                        }
                    }
                    return true;
                }

                static bool DontCare(const Group &group)
                {
                    return true;
                }

                static bool CareTooMuch(const Group &group)
                {
                    return false;
                }

                static bool None(const Group &group)
                {
                    return group.MatchedChildren() == 0;
                }
            };
    };

    // Command line argument parser
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
                    group("arguments", Group::Validators::AllChildGroups) {}

            void Add(Base &child)
            {
                group.Add(child);
            }

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

                        FlagBase *base = group.Match(arg);
                        if (base)
                        {
                            ArgFlagBase *argbase = dynamic_cast<ArgFlagBase *>(base);
                            if (argbase)
                            {
                                if (separator != argchunk.npos)
                                {
                                    argbase->ParseArg(argchunk.substr(separator + longseparator.size()));
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
                                        argbase->ParseArg(*it);
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
                            problem << "Argument could not be matched: " << arg;
                            throw ParseError(problem.str().c_str());
                        }
                        // Check short args
                    } else if (chunk.find(shortprefix) == 0 && chunk.size() > shortprefix.size())
                    {
                        std::string argchunk(chunk.substr(shortprefix.size()));
                        for (auto argit = std::begin(argchunk); argit != std::end(argchunk); ++argit)
                        {
                            const char arg = *argit;

                            Base *base = group.Match(arg);
                            if (base)
                            {
                                ArgFlagBase *argbase = dynamic_cast<ArgFlagBase *>(base);
                                if (argbase)
                                {
                                    argchunk.erase(std::begin(argchunk), ++argit);
                                    if (!argchunk.empty())
                                    {
                                        argbase->ParseArg(argchunk);
                                    } else
                                    {
                                        ++it;
                                        if (it == std::end(args))
                                        {
                                            std::ostringstream problem;
                                            problem << "Flag '" << arg << "' requires an argument but received none";
                                            throw ParseError(problem.str().c_str());
                                        } else
                                        {
                                            argbase->ParseArg(*it);
                                        }
                                    }
                                    // Because this argchunk is done regardless
                                    break;
                                }
                            } else
                            {
                                std::ostringstream problem;
                                problem << "Argument could not be matched: '" << arg << "'";
                                throw ParseError(problem.str().c_str());
                            }
                        }
                    } else
                    {
                        SetNextPositional(chunk);
                    }
                }
                if (!group.Matched())
                {
                    std::ostringstream problem;
                    problem << "Group validation failed somewhere!";
                    throw ValidationError(problem.str().c_str());
                }
            }

            void SetNextPositional(const std::string &arg)
            {
                PosBase *pos = group.GetNextPos();
                if (pos)
                {
                    pos->ParseArg(arg);
                } else
                {
                    std::ostringstream problem;
                    problem << "Passed in argument, but no positional arguments were ready to receive it" << arg;
                    throw ParseError(problem.str().c_str());
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
    class Flag : public FlagBase
    {
        public:
            Flag(Group &group, const std::string &name, const std::string &help, const Matcher &matcher): FlagBase(name, help, matcher)
            {
                group.Add(*this);
            }

            Flag(Group &group, const std::string &name, const std::string &help, Matcher &&matcher): FlagBase(name, help, std::move(matcher))
            {
                group.Add(*this);
            }

            virtual ~Flag() {}

    };

    // Help flag class
    class HelpFlag : public Flag
    {
        public:
            HelpFlag(Group &group, const std::string &name, const std::string &help, const Matcher &matcher): Flag(group, name, help, matcher) {}
            HelpFlag(Group &group, const std::string &name, const std::string &help, Matcher &&matcher): Flag(group, name, help, std::move(matcher)) {}

            virtual ~HelpFlag() {}

            virtual FlagBase *Match(const std::string &arg) override
            {
                if (FlagBase::Match(arg))
                {
                    throw Help(arg);
                }
                return nullptr;
            }

            virtual FlagBase *Match(const char arg) override
            {
                if (FlagBase::Match(arg))
                {
                    throw Help(std::string(1, arg));
                }
                return nullptr;
            }
    };

    // Count matches
    class Counter : public FlagBase
    {
        private:
            unsigned int count;

        public:
            Counter(Group &group, const std::string &name, const std::string &help, const Matcher &matcher, const unsigned int startcount = 0): FlagBase(name, help, matcher), count(startcount)
            {
                group.Add(*this);
            }

            Counter(Group &group, const std::string &name, const std::string &help, Matcher &&matcher, const unsigned int startcount = 0): FlagBase(name, help, std::move(matcher)), count(startcount)
            {
                group.Add(*this);
            }

            virtual ~Counter() {}

            virtual FlagBase *Match(const std::string &arg) override
            {
                FlagBase *me = FlagBase::Match(arg);
                if (me)
                {
                    ++count;
                }
                return me;
            }

            virtual FlagBase *Match(const char arg) override
            {
                FlagBase *me = FlagBase::Match(arg);
                if (me)
                {
                    ++count;
                }
                return me;
            }

            unsigned int Count()
            {
                return count;
            }
    };

    template <typename T>
    void ArgReader(const std::string &name, const std::string &value, T &destination)
    {
        std::istringstream ss(value);
        ss >> destination;

        if (ss.rdbuf()->in_avail() > 0)
        {
            std::ostringstream problem;
            problem << "Argument '" << name << "' received invalid value type '" << value << "'";
            throw ParseError(problem.str().c_str());
        }
    }

    template <>
    void ArgReader<std::string>(const std::string &name, const std::string &value, std::string &destination)
    {
        destination.assign(value);
    }

    template <typename T, void (*Reader)(const std::string &, const std::string &, T&) = ArgReader<T>>
    class ArgFlag : public ArgFlagBase
    {
        private:
            T value;

        public:
            ArgFlag(Group &group, const std::string &name, const std::string &help, const Matcher &matcher): ArgFlagBase(name, help, matcher)
            {
                group.Add(*this);
            }

            ArgFlag(Group &group, const std::string &name, const std::string &help, Matcher &&matcher): ArgFlagBase(name, help, std::move(matcher))
            {
                group.Add(*this);
            }

            virtual ~ArgFlag() {}

            virtual void ParseArg(const std::string &value) override
            {
                Reader(name, value, this->value);
            }

            const T &Value()
            {
                return value;
            }
    };

    template <
        typename T,
        typename List = std::vector<T>,
        void (*Reader)(const std::string &, const std::string &, T&) = ArgReader<T>>
    class ArgFlagList : public ArgFlagBase
    {
        private:
            List values;

        public:
            ArgFlagList(Group &group, const std::string &name, const std::string &help, const Matcher &matcher): ArgFlagBase(name, help, matcher)
            {
                group.Add(*this);
            }

            ArgFlagList(Group &group, const std::string &name, const std::string &help, Matcher &&matcher): ArgFlagBase(name, help, std::move(matcher))
            {
                group.Add(*this);
            }

            virtual ~ArgFlagList() {}

            virtual void ParseArg(const std::string &value) override
            {
                values.emplace_back();
                Reader(name, value, values.back());
            }

            const List &Values()
            {
                return values;
            }
    };

    template <typename T, void (*Reader)(const std::string &, const std::string &, T&) = ArgReader<T>>
    class PosArg : public PosBase
    {
        private:
            T value;

        public:
            PosArg(Group &group, const std::string &name, const std::string &help): PosBase(name, help)
            {
                group.Add(*this);
            }

            virtual ~PosArg() {}

            virtual void ParseArg(const std::string &value) override
            {
                Reader(name, value, this->value);
                ready = false;
                matched = true;
            }

            const T &Value()
            {
                return value;
            }
    };

    template <
        typename T,
        typename List = std::vector<T>,
        void (*Reader)(const std::string &, const std::string &, T&) = ArgReader<T>>
    class PosArgList : public PosBase
    {
        private:
            List values;

        public:
            PosArgList(Group &group, const std::string &name, const std::string &help): PosBase(name, help)
            {
                group.Add(*this);
            }

            virtual ~PosArgList() {}

            virtual void ParseArg(const std::string &value) override
            {
                values.emplace_back();
                Reader(name, value, values.back());
                matched = true;
            }

            const List &Values()
            {
                return values;
            }
    };
}
