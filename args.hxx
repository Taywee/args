/* Copyright © 2016 Taylor C. Richberger <taywee@gmx.com>
 * This code is released under the license described in the LICENSE file
 */

#include <algorithm>
#include <exception>
#include <functional>
#include <locale>
#include <sstream>
#include <string>
#include <tuple>
#include <vector>

namespace args
{
    // Wrap a string into a vector of string lines
    std::vector<std::string> Wrap(const std::string &in, const size_t width)
    {
        // Preserve existing line breaks
        const size_t newlineloc = in.find('\n');
        if (newlineloc != in.npos)
        {
            std::vector<std::string> first(Wrap(std::string(in, 0, newlineloc), width));
            std::vector<std::string> second(Wrap(std::string(in, newlineloc + 1), width));
            first.insert(
                std::end(first),
                std::make_move_iterator(std::begin(second)),
                std::make_move_iterator(std::end(second)));
            return first;
        }
        std::istringstream stream(in);
        std::vector<std::string> output;
        std::ostringstream line;
        while (stream)
        {
            std::string item;
            stream >> item;
            if ((size_t(line.tellp()) + 1 + item.size()) > width)
            {
                if (line.tellp() > 0)
                {
                    output.push_back(line.str());
                    line.str(std::string());
                }
            }
            if (line.tellp() > 0)
            {
                line << " ";
            }
            line << item;
        }
        if (line.tellp() > 0)
        {
            output.push_back(line.str());
        }
        return output;
    }

    std::string GetArgName(std::string string)
    {
        std::locale loc;
        for (unsigned int i = 0; i < string.size(); ++i)
        {
            const char c = string[i];
            switch (c)
            {
                case ' ':
                case '-':
                    string[i] = '_';
                    break;

                default:
                    string[i] = std::toupper(string[i], loc);
                    break;
            }
        }
        return string;
    }

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

            std::vector<std::string> GetOptionStrings(const std::string &shortPrefix, const std::string &longPrefix) const
            {
                std::vector<std::string> optStrings;
                optStrings.reserve(shortOpts.size() + longOpts.size());
                for (const char opt: shortOpts)
                {
                    optStrings.emplace_back(shortPrefix + std::string(1, opt));
                }
                for (const std::string &opt: longOpts)
                {
                    optStrings.emplace_back(longPrefix + opt);
                }
                return optStrings;
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

            virtual std::tuple<std::string, std::string> GetDescription(const std::string &shortPrefix, const std::string &longPrefix) const
            {
                std::tuple<std::string, std::string> description;
                std::get<1>(description) = help;
                return description;
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

            virtual std::tuple<std::string, std::string> GetDescription(const std::string &shortPrefix, const std::string &longPrefix) const override
            {
                std::tuple<std::string, std::string> description;
                std::get<0>(description) = GetArgName(name);
                std::get<1>(description) = help;
                return description;
            }

    };

    // Base class for flag arguments
    class FlagBase : public NamedBase
    {
        protected:
            const Matcher matcher;

        public:
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

            virtual std::tuple<std::string, std::string> GetDescription(const std::string &shortPrefix, const std::string &longPrefix) const override
            {
                std::tuple<std::string, std::string> description;
                const std::string upperName(GetArgName(name));
                const std::vector<std::string> optStrings(matcher.GetOptionStrings(shortPrefix, longPrefix));
                std::ostringstream flagstream;
                for (auto it = std::begin(optStrings); it != std::end(optStrings); ++it)
                {
                    if (it != std::begin(optStrings))
                    {
                        flagstream << ", ";
                    }
                    flagstream << *it;
                }
                std::get<0>(description) = flagstream.str();
                std::get<1>(description) = help;
                return description;
            }
    };

    // Base class that takes arguments
    class ArgFlagBase : public FlagBase
    {
        public:
            ArgFlagBase(const std::string &name, const std::string &help, Matcher &&matcher) : FlagBase(name, help, std::move(matcher)) {}
            virtual ~ArgFlagBase() {}
            virtual void ParseArg(const std::string &value) = 0;

            virtual std::tuple<std::string, std::string> GetDescription(const std::string &shortPrefix, const std::string &longPrefix) const override
            {
                std::tuple<std::string, std::string> description;
                const std::string upperName(GetArgName(name));
                const std::vector<std::string> optStrings(matcher.GetOptionStrings(shortPrefix, longPrefix));
                std::ostringstream flagstream;
                for (auto it = std::begin(optStrings); it != std::end(optStrings); ++it)
                {
                    if (it != std::begin(optStrings))
                    {
                        flagstream << ", ";
                    }
                    flagstream << *it << ' ' << upperName;
                }
                std::get<0>(description) = flagstream.str();
                std::get<1>(description) = help;
                return description;
            }
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
            Group(Group &group, const std::string &help, const std::function<bool(const Group &)> &validator = Validators::DontCare) : Base(help), validator(validator)
            {
                group.Add(*this);
            }
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

            std::vector<std::tuple<std::string, std::string>> GetChildDescriptions(const std::string &shortPrefix, const std::string &longPrefix) const
            {
                std::vector<std::tuple<std::string, std::string>> descriptions;
                for (const auto &child: children)
                {
                    const Group *group = dynamic_cast<Group *>(child);
                    const NamedBase *named = dynamic_cast<NamedBase *>(child);
                    if (group)
                    {
                        std::vector<std::tuple<std::string, std::string>> groupDescriptions(group->GetChildDescriptions(shortPrefix, longPrefix));
                        descriptions.insert(
                            std::end(descriptions),
                            std::make_move_iterator(std::begin(groupDescriptions)),
                            std::make_move_iterator(std::end(groupDescriptions)));
                    } else if (named)
                    {
                        descriptions.emplace_back(named->GetDescription(shortPrefix, longPrefix));
                    }
                }
                return descriptions;
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
    class ArgumentParser : public Group
    {
        private:
            std::string prog;
            std::string description;
            std::string epilog;

            std::string longprefix;
            std::string shortprefix;

            std::string longseparator;

        public:
            ArgumentParser(const std::string &description, const std::string &epilog = std::string()) :
                Group("arguments", Group::Validators::AllChildGroups),
                description(description),
                epilog(epilog),
                longprefix("--"),
                shortprefix("-"),
                longseparator("=") {}

            // Ugly getter/setter section
            const std::string &Prog() const
            { return prog; }
            void Prog(const std::string &prog)
            { this->prog = prog; }

            const std::string &Description() const
            { return description; }
            void Description(const std::string &description)
            { this->description = description; }
            
            const std::string &Epilog() const
            { return epilog; }
            void Epilog(const std::string &epilog)
            { this->epilog = epilog; }

            const std::string &LongPrefix() const
            { return longprefix; }
            void LongPrefix(const std::string &longprefix)
            { this->longprefix = longprefix; }

            const std::string &ShortPrefix() const
            { return shortprefix; }
            void ShortPrefix(const std::string &shortprefix)
            { this->shortprefix = shortprefix; }

            const std::string &LongSeparator() const
            { return longseparator; }
            void LongSeparator(const std::string &longseparator)
            { this->longseparator = longseparator; }
            std::string Help(unsigned int width = 80, unsigned int progindent = 2, unsigned int descriptionindent = 4, unsigned int flagindent = 6, unsigned int helpindent = 40, unsigned int gutter = 1) const
            {
                const std::vector<std::string> description(Wrap(this->description, width - descriptionindent));
                const std::vector<std::string> epilog(Wrap(this->epilog, width - descriptionindent));
                std::ostringstream help;
                help << std::string(progindent, ' ') << prog << "\n\n";
                for (const std::string &line: description)
                {
                    help << std::string(descriptionindent, ' ') << line << "\n";
                }
                help << "\n";
                help << std::string(progindent, ' ') << "OPTIONS:\n\n";
                for (const auto &description: GetChildDescriptions(shortprefix, longprefix))
                {
                    const std::string &flags = std::get<0>(description);
                    const std::vector<std::string> info(Wrap(std::get<1>(description), width - helpindent));
                    help << std::string(flagindent, ' ') << flags;
                    auto infoit = std::begin(info);
                    if ((flagindent + flags.size() + gutter) > helpindent)
                    {
                        help << '\n';
                    } else if (infoit != std::end(info))
                    {
                        help << std::string(helpindent - (flagindent + flags.size()), ' ') << *infoit << '\n';
                        ++infoit;
                    }
                    for (; infoit != std::end(info); ++infoit)
                    {
                        help << std::string(helpindent, ' ') << *infoit << "\n";
                    }
                }

                help << "\n";
                for (const std::string &line: epilog)
                {
                    help << std::string(descriptionindent, ' ') << line << "\n";
                }
                return help.str();
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

                        FlagBase *base = Match(arg);
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

                            Base *base = Match(arg);
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
                if (!Matched())
                {
                    std::ostringstream problem;
                    problem << "Group validation failed somewhere!";
                    throw ValidationError(problem.str().c_str());
                }
            }

            void SetNextPositional(const std::string &arg)
            {
                PosBase *pos = GetNextPos();
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
            ArgFlag(Group &group, const std::string &name, const std::string &help, Matcher &&matcher, const T &defaultValue = T()): ArgFlagBase(name, help, std::move(matcher)), value(defaultValue)
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
            ArgFlagList(Group &group, const std::string &name, const std::string &help, Matcher &&matcher, const List &defaultValues = List()): ArgFlagBase(name, help, std::move(matcher)), values(defaultValues)
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
            PosArg(Group &group, const std::string &name, const std::string &help, const T &defaultValue = T()): PosBase(name, help), value(defaultValue)
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
            PosArgList(Group &group, const std::string &name, const std::string &help, const List &defaultValues = List()): PosBase(name, help), values(defaultValues)
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
