/* Copyright (c) 2016 Taylor C. Richberger <taywee@gmx.com>
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include <algorithm>
#include <exception>
#include <functional>
#include <sstream>
#include <string>
#include <tuple>
#include <vector>
#include <unordered_set>

namespace args
{
    /** (INTERNAL) Count UTF-8 glyphs
     *
     * This is not reliable, and will fail for combinatory glyphs, but it's
     * good enough here for now.
     *
     * \param string The string to count glyphs from
     * \return The UTF-8 glyphs in the string
     */
    std::string::size_type Glyphs(const std::string &string)
    {
        std::string::size_type length = 0;
        for (const char c: string)
        {
            if ((c & 0xc0) != 0x80)
            {
                ++length;
            }
        }
        return length;
    }

    /** (INTERNAL) Wrap a string into a vector of lines
     *
     * This is quick and hacky, but works well enough.  You can specify a
     * different width for the first line
     *
     * \param width The width of the body
     * \param the widtho f the first line, defaults to the width of the body
     * \return the vector of lines
     */
    std::vector<std::string> Wrap(const std::string &in, const std::string::size_type width, std::string::size_type firstlinewidth = 0)
    {
        // Preserve existing line breaks
        const std::string::size_type newlineloc = in.find('\n');
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
        if (firstlinewidth == 0)
        {
            firstlinewidth = width;
        }
        std::string::size_type currentwidth = firstlinewidth;

        std::istringstream stream(in);
        std::vector<std::string> output;
        std::ostringstream line;
        std::string::size_type linesize = 0;
        while (stream)
        {
            std::string item;
            stream >> item;
            std::string::size_type itemsize = Glyphs(item);
            if ((linesize + 1 + itemsize) > currentwidth)
            {
                if (linesize > 0)
                {
                    output.push_back(line.str());
                    line.str(std::string());
                    linesize = 0;
                    currentwidth = width;
                }
            }
            if (itemsize > 0)
            {
                if (linesize)
                {
                    ++linesize;
                    line << " ";
                }
                line << item;
                linesize += itemsize;
            }
        }
        if (linesize > 0)
        {
            output.push_back(line.str());
        }
        return output;
    }

    /** Errors that occur during regular parsing
     */
    class ParseError : public std::runtime_error
    {
        public:
            ParseError(const char *problem) : std::runtime_error(problem) {}
            virtual ~ParseError() {};
    };

    /** Errors that are detected from group validation after parsing finishes
     */
    class ValidationError : public std::runtime_error
    {
        public:
            ValidationError(const char *problem) : std::runtime_error(problem) {}
            virtual ~ValidationError() {};
    };

    /** An exception that indicates that the user has requested help
     */
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

    /** A simple unified option type for unified initializer lists for the Matcher class.
     */
    struct EitherOpt
    {
        const bool isShort;
        const char shortOpt;
        const std::string longOpt;
        EitherOpt(const std::string &opt) : isShort(false), shortOpt(), longOpt(opt) {}
        EitherOpt(const char *opt) : isShort(false), shortOpt(), longOpt(opt) {}
        EitherOpt(const char opt) : isShort(true), shortOpt(opt), longOpt() {}

        /** Get just the long options from an initializer list of EitherOpts
         */
        static std::unordered_set<std::string> GetLong(std::initializer_list<EitherOpt> opts)
        {
            std::unordered_set<std::string>  longOpts;
            for (const EitherOpt &opt: opts)
            {
                if (!opt.isShort)
                {
                    longOpts.insert(opt.longOpt);
                }
            }
            return longOpts;
        }

        /** Get just the short options from an initializer list of EitherOpts
         */
        static std::unordered_set<char> GetShort(std::initializer_list<EitherOpt> opts)
        {
            std::unordered_set<char>  shortOpts;
            for (const EitherOpt &opt: opts)
            {
                if (opt.isShort)
                {
                    shortOpts.insert(opt.shortOpt);
                }
            }
            return shortOpts;
        }
    };



    /** A class of "matchers", specifying short and long options that can
     * possibly be matched.
     *
     * This is supposed to be constructed and then passed in, not used directly
     * from user code.
     */
    class Matcher
    {
        private:
            const std::unordered_set<char> shortOpts;
            const std::unordered_set<std::string> longOpts;

        public:
            /** Specify short and long opts separately as iterators
             *
             * ex: `args::Matcher(shortOpts.begin(), shortOpts.end(), longOpts.begin(), longOpts.end())`
             */
            template <typename ShortIt, typename LongIt>
            Matcher(ShortIt shortOptsStart, ShortIt shortOptsEnd, LongIt longOptsStart, LongIt longOptsEnd) :
                shortOpts(shortOptsStart, shortOptsEnd),
                longOpts(longOptsStart, longOptsEnd)
            {}

            /** Specify short and long opts separately as iterables
             *
             * ex: `args::Matcher(shortOpts, longOpts)`
             */
            template <typename Short, typename Long>
            Matcher(Short &&shortIn, Long &&longIn) :
                shortOpts(std::begin(shortIn), std::end(shortIn)), longOpts(std::begin(longIn), std::end(longIn))
            {}

            /** Specify a mixed single initializer-list of both short and long opts
             *
             * This is the fancy one.  It takes a single initializer list of
             * any number of any mixed kinds of options.  Chars are
             * automatically interpreted as short options, and strings are
             * automatically interpreted as long options:
             *
             *     args::Matcher{'a'}
             *     args::Matcher{"foo"}
             *     args::Matcher{'h', "help"}
             *     args::Matcher{"foo", 'f', 'F', "FoO"}
             */
            Matcher(std::initializer_list<EitherOpt> in) :
                shortOpts(EitherOpt::GetShort(in)), longOpts(EitherOpt::GetLong(in)) {}

            Matcher(Matcher &&other) : shortOpts(std::move(other.shortOpts)), longOpts(std::move(other.longOpts))
            {}

            ~Matcher() {}

            /** (INTERNAL) Check if there is a match of a short opt
             */
            bool Match(const char opt) const
            {
                return shortOpts.find(opt) != shortOpts.end();
            }

            /** (INTERNAL) Check if there is a match of a long opt
             */
            bool Match(const std::string &opt) const
            {
                return longOpts.find(opt) != longOpts.end();
            }

            /** (INTERNAL) Get all option strings as a vector, with the prefixes embedded
             */
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

            /** (INTERNAL) Get all option strings as a vector, with the prefixes and names embedded
             */
            std::vector<std::string> GetOptionStrings(const std::string &shortPrefix, const std::string &longPrefix, const std::string &name, const std::string &shortSeparator, const std::string longSeparator) const
            {
                const std::string bracedname(std::string("[") + name + "]");
                std::vector<std::string> optStrings;
                optStrings.reserve(shortOpts.size() + longOpts.size());
                for (const char opt: shortOpts)
                {
                    optStrings.emplace_back(shortPrefix + std::string(1, opt) + shortSeparator + bracedname);
                }
                for (const std::string &opt: longOpts)
                {
                    optStrings.emplace_back(longPrefix + opt + longSeparator + bracedname);
                }
                return optStrings;
            }
    };

    /** Base class for all match types
     */
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

            virtual std::tuple<std::string, std::string> GetDescription(const std::string &shortPrefix, const std::string &longPrefix, const std::string &shortSeparator, const std::string &longSeparator) const
            {
                std::tuple<std::string, std::string> description;
                std::get<1>(description) = help;
                return description;
            }

            virtual void ResetMatched()
            {
                matched = false;
            }
    };

    /** Base class for all match types that have a name
     */
    class NamedBase : public Base
    {
        protected:
            const std::string name;

        public:
            NamedBase(const std::string &name, const std::string &help) : Base(help), name(name) {}
            virtual ~NamedBase() {}

            virtual std::tuple<std::string, std::string> GetDescription(const std::string &shortPrefix, const std::string &longPrefi, const std::string &shortSeparator, const std::string &longSeparator) const override
            {
                std::tuple<std::string, std::string> description;
                std::get<0>(description) = name;
                std::get<1>(description) = help;
                return description;
            }
            virtual std::string Name() const
            {
                return name;
            }
    };

    /** Base class for all flag arguments
     */
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

            virtual std::tuple<std::string, std::string> GetDescription(const std::string &shortPrefix, const std::string &longPrefix, const std::string &shortSeparator, const std::string &longSeparator) const override
            {
                std::tuple<std::string, std::string> description;
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

    /** Base class for argument-accepting flag arguments
     */
    class ArgFlagBase : public FlagBase
    {
        public:
            ArgFlagBase(const std::string &name, const std::string &help, Matcher &&matcher) : FlagBase(name, help, std::move(matcher)) {}
            virtual ~ArgFlagBase() {}
            virtual void ParseArg(const std::string &value) = 0;

            virtual std::tuple<std::string, std::string> GetDescription(const std::string &shortPrefix, const std::string &longPrefix, const std::string &shortSeparator, const std::string &longSeparator) const override
            {
                std::tuple<std::string, std::string> description;
                const std::vector<std::string> optStrings(matcher.GetOptionStrings(shortPrefix, longPrefix, name, shortSeparator, longSeparator));
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

    /** Base class for positional arguments
     */
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

    /** Class for all kinds of validating groups, including ArgumentParser
     */
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

            /** Return the first FlagBase that matches arg, or nullptr
             *
             * \param arg The argument with prefixes stripped
             * \return the first matching FlagBase pointer, or nullptr if there is no match
             */
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

            /** Return the first FlagBase that matches arg, or nullptr
             *
             * \param arg The argument with prefixes stripped
             * \return the first matching FlagBase pointer, or nullptr if there is no match
             */
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

            /** Get the next ready positional parameter, or nullptr if there is none
             *
             * \return the first ready PosBase pointer, or nullptr if there is no match
             */
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

            /** Get whether this has any FlagBase children
             *
             * \return Whether or not there are any FlagBase children
             */
            bool HasFlag() const
            {
                for (Base *child: children)
                {
                    FlagBase *flag = dynamic_cast<FlagBase *>(child);
                    Group *group = dynamic_cast<Group *>(child);
                    if (flag)
                    {
                        return true;
                    }
                    if (group)
                    {
                        if (group->HasFlag())
                        {
                            return true;
                        }
                    }
                }
                return false;
            }

            /** Append a child to this Group.
             */
            void Add(Base &child)
            {
                children.emplace_back(&child);
            }

            /** Get all this group's children
             */
            const std::vector<Base *> Children() const
            {
                return children;
            }

            /** Count the number of matched children this group has
             */
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

            /** Whether or not this group matches validation
             */
            virtual bool Matched() const noexcept override
            {
                return validator(*this);
            }

            /** Get all the child descriptions for help generation
             */
            std::vector<std::tuple<std::string, std::string, unsigned int>> GetChildDescriptions(const std::string &shortPrefix, const std::string &longPrefix, const std::string &shortSeparator, const std::string &longSeparator, unsigned int indent = 0) const
            {
                std::vector<std::tuple<std::string, std::string, unsigned int>> descriptions;
                for (const auto &child: children)
                {
                    const Group *group = dynamic_cast<Group *>(child);
                    const NamedBase *named = dynamic_cast<NamedBase *>(child);
                    if (group)
                    {
                        // Push that group description on the back:
                        descriptions.emplace_back(group->help, "", indent);
                        std::vector<std::tuple<std::string, std::string, unsigned int>> groupDescriptions(group->GetChildDescriptions(shortPrefix, longPrefix, shortSeparator, longSeparator, indent + 1));
                        descriptions.insert(
                            std::end(descriptions),
                            std::make_move_iterator(std::begin(groupDescriptions)),
                            std::make_move_iterator(std::end(groupDescriptions)));
                    } else if (named)
                    {
                        const std::tuple<std::string, std::string> description(named->GetDescription(shortPrefix, longPrefix, shortSeparator, longSeparator));
                        descriptions.emplace_back(std::get<0>(description), std::get<1>(description), indent);
                    }
                }
                return descriptions;
            }

            /** Get the names of positional parameters
             */
            std::vector<std::string> GetPosNames() const
            {
                std::vector <std::string> names;
                for (const auto &child: children)
                {
                    const Group *group = dynamic_cast<Group *>(child);
                    const PosBase *pos = dynamic_cast<PosBase *>(child);
                    if (group)
                    {
                        std::vector<std::string> groupNames(group->GetPosNames());
                        names.insert(
                            std::end(names),
                            std::make_move_iterator(std::begin(groupNames)),
                            std::make_move_iterator(std::end(groupNames)));
                    } else if (pos)
                    {
                        names.emplace_back(pos->Name());
                    }
                }
                return names;
            }

            virtual void ResetMatched() override
            {
                for (auto &child: children)
                {
                    child->ResetMatched();
                }
            }

            /** Default validators
             */
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

    /** The main user facing command line argument parser class
     */
    class ArgumentParser : public Group
    {
        private:
            std::string prog;
            std::string description;
            std::string epilog;

            std::string longprefix;
            std::string shortprefix;

            std::string longseparator;

            std::string terminator;

            bool allowJoinedShortArgument;
            bool allowJoinedLongArgument;
            bool allowSeparateShortArgument;
            bool allowSeparateLongArgument;

        public:
            /** A simple structure of parameters for easy user-modifyable help menus
             */
            struct HelpParams
            {
                /** The width of the help menu
                 */
                unsigned int width = 80;
                /** The indent of the program line
                 */
                unsigned int progindent = 2;
                /** The indent of the program trailing lines for long parameters
                 */
                unsigned int progtailindent = 4;
                /** The indent of the description and epilogs
                 */
                unsigned int descriptionindent = 4;
                /** The indent of the flags
                 */
                unsigned int flagindent = 6;
                /** The indent of the flag descriptions
                 */
                unsigned int helpindent = 40;
                /** The additional indent each group adds
                 */
                unsigned int eachgroupindent = 2;

                /** The minimum gutter between each flag and its help
                 */
                unsigned int gutter = 1;
            } helpParams;
            ArgumentParser(const std::string &description, const std::string &epilog = std::string()) :
                Group("arguments", Group::Validators::AllChildGroups),
                description(description),
                epilog(epilog),
                longprefix("--"),
                shortprefix("-"),
                longseparator("="),
                terminator("--"),
                allowJoinedShortArgument(true),
                allowJoinedLongArgument(true),
                allowSeparateShortArgument(true),
                allowSeparateLongArgument(true) {}

            /** The program name for help generation
             */
            const std::string &Prog() const
            { return prog; }
            /** The program name for help generation
             */
            void Prog(const std::string &prog)
            { this->prog = prog; }

            /** The description that appears above options
             */
            const std::string &Description() const
            { return description; }
            /** The description that appears above options
             */
            void Description(const std::string &description)
            { this->description = description; }
            
            /** The description that appears below options
             */
            const std::string &Epilog() const
            { return epilog; }
            /** The description that appears below options
             */
            void Epilog(const std::string &epilog)
            { this->epilog = epilog; }

            /** The prefix for long options
             */
            const std::string &LongPrefix() const
            { return longprefix; }
            /** The prefix for long options
             */
            void LongPrefix(const std::string &longprefix)
            { this->longprefix = longprefix; }

            /** The prefix for short options
             */
            const std::string &ShortPrefix() const
            { return shortprefix; }
            /** The prefix for short options
             */
            void ShortPrefix(const std::string &shortprefix)
            { this->shortprefix = shortprefix; }

            /** The separator for long options
             */
            const std::string &LongSeparator() const
            { return longseparator; }
            /** The separator for long options
             */
            void LongSeparator(const std::string &longseparator)
            {
                if (longseparator.empty())
                {
                    throw std::runtime_error("longseparator can not be set to empty");
                }
                this->longseparator = longseparator;
            }

            /** The terminator that separates short options from long ones
             */
            const std::string &Terminator() const
            { return terminator; }
            /** The terminator that separates short options from long ones
             */
            void Terminator(const std::string &terminator)
            { this->terminator = terminator; }

            /** Get the current argument separation parameters.
             *
             * See SetArgumentSeparations for details on what each one means.
             */
            void GetArgumentSeparations(
                bool &allowJoinedShortArgument,
                bool &allowJoinedLongArgument,
                bool &allowSeparateShortArgument,
                bool &allowSeparateLongArgument) const
            {
                allowJoinedShortArgument = this->allowJoinedShortArgument;
                allowJoinedLongArgument = this->allowJoinedLongArgument;
                allowSeparateShortArgument = this->allowSeparateShortArgument;
                allowSeparateLongArgument = this->allowSeparateLongArgument;
            }

            /** Change allowed option separation.
             *
             * \param allowJoinedShortArgument Allow a short flag that accepts an argument to be passed its argument immediately next to it (ie. in the same argv field)
             * \param allowJoinedLongArgument Allow a long flag that accepts an argument to be passed its argument separated by the longseparator (ie. in the same argv field)
             * \param allowSeparateShortArgument Allow a short flag that accepts an argument to be passed its argument separated by whitespace (ie. in the next argv field)
             * \param allowSeparateLongArgument Allow a long flag that accepts an argument to be passed its argument separated by whitespace (ie. in the next argv field)
             */
            void SetArgumentSeparations(
                const bool allowJoinedShortArgument,
                const bool allowJoinedLongArgument,
                const bool allowSeparateShortArgument,
                const bool allowSeparateLongArgument)
            {
                this->allowJoinedShortArgument = allowJoinedShortArgument;
                this->allowJoinedLongArgument = allowJoinedLongArgument;
                this->allowSeparateShortArgument = allowSeparateShortArgument;
                this->allowSeparateLongArgument = allowSeparateLongArgument;
            }

            /** Pass the help menu into an ostream
             */
            void Help(std::ostream &help) const
            {
                bool hasoptions = false;
                bool hasarguments = false;

                const std::vector<std::string> description(Wrap(this->description, helpParams.width - helpParams.descriptionindent));
                const std::vector<std::string> epilog(Wrap(this->epilog, helpParams.width - helpParams.descriptionindent));
                std::ostringstream prognameline;
                prognameline << prog;
                if (HasFlag())
                {
                    hasoptions = true;
                    prognameline << " {OPTIONS}";
                }
                for (const std::string &posname: GetPosNames())
                {
                    hasarguments = true;
                    prognameline << " [" << posname << ']';
                }
                const std::vector<std::string> proglines(Wrap(prognameline.str(), helpParams.width - (helpParams.progindent + 4), helpParams.width - helpParams.progindent));
                auto progit = std::begin(proglines);
                if (progit != std::end(proglines))
                {
                    help << std::string(helpParams.progindent, ' ') << *progit << '\n';
                    ++progit;
                }
                for (; progit != std::end(proglines); ++progit)
                {
                    help << std::string(helpParams.progtailindent, ' ') << *progit << '\n';
                }

                help << '\n';

                for (const std::string &line: description)
                {
                    help << std::string(helpParams.descriptionindent, ' ') << line << "\n";
                }
                help << "\n";
                help << std::string(helpParams.progindent, ' ') << "OPTIONS:\n\n";
                for (const auto &description: GetChildDescriptions(shortprefix, longprefix, allowJoinedShortArgument ? "" : " ", allowJoinedLongArgument ? longseparator : " "))
                {
                    const unsigned int groupindent = std::get<2>(description) * helpParams.eachgroupindent;
                    const std::vector<std::string> flags(Wrap(std::get<0>(description), helpParams.width - (helpParams.flagindent + helpParams.helpindent + helpParams.gutter)));
                    const std::vector<std::string> info(Wrap(std::get<1>(description), helpParams.width - (helpParams.helpindent + groupindent)));

                    std::string::size_type flagssize = 0;
                    for (auto flagsit = std::begin(flags); flagsit != std::end(flags); ++flagsit)
                    {
                        if (flagsit != std::begin(flags))
                        {
                            help << '\n';
                        }
                        help << std::string(groupindent + helpParams.flagindent, ' ') << *flagsit;
                        flagssize = Glyphs(*flagsit);
                    }

                    auto infoit = std::begin(info);
                    // groupindent is on both sides of this inequality, and therefore can be removed
                    if ((helpParams.flagindent + flagssize + helpParams.gutter) > helpParams.helpindent || infoit == std::end(info))
                    {
                        help << '\n';
                    } else
                    {
                        // groupindent is on both sides of the minus sign, and therefore doesn't actually need to be in here
                        help << std::string(helpParams.helpindent - (helpParams.flagindent + flagssize), ' ') << *infoit << '\n';
                        ++infoit;
                    }
                    for (; infoit != std::end(info); ++infoit)
                    {
                        help << std::string(groupindent + helpParams.helpindent, ' ') << *infoit << '\n';
                    }
                }
                if (hasoptions && hasarguments)
                {
                    for (const std::string &item: Wrap(std::string("\"") + terminator + "\" can be used to terminate flag options and force all following arguments to be treated as positional options", helpParams.width - helpParams.flagindent))
                    {
                        help << std::string(helpParams.flagindent, ' ') << item << '\n';
                    }
                }

                help << "\n";
                for (const std::string &line: epilog)
                {
                    help << std::string(helpParams.descriptionindent, ' ') << line << "\n";
                }
            }

            /** Generate a help menu as a string.
             *
             * \return the help text as a single string
             */
            std::string Help() const
            {
                std::ostringstream help;
                Help(help);
                return help.str();
            }

            /** Parse all arguments.
             *
             * \param begin an iterator to the beginning of the argument list
             * \param end an iterator to the past-the-end element of the argument list
             */
            template <typename It>
            void ParseArgs(It begin, It end)
            {
                // Reset all Matched statuses to false, for validation.  Don't reset values.
                ResetMatched();
                bool terminated = false;

                // Check all arg chunks
                for (auto it = begin; it != end; ++it)
                {
                    const std::string &chunk = *it;

                    if (!terminated and chunk == terminator)
                    {
                        terminated = true;
                    // If a long arg was found
                    } else if (!terminated && chunk.find(longprefix) == 0 && chunk.size() > longprefix.size())
                    {
                        const std::string argchunk(chunk.substr(longprefix.size()));
                        // Try to separate it, in case of a separator:
                        const auto separator = longseparator.empty() ? argchunk.npos : argchunk.find(longseparator);
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
                                    if (allowJoinedLongArgument)
                                    {
                                        argbase->ParseArg(argchunk.substr(separator + longseparator.size()));
                                    } else
                                    {
                                        std::ostringstream problem;
                                        problem << "Flag '" << arg << "' was passed a joined argument, but these are disallowed";
                                        throw ParseError(problem.str().c_str());
                                    }
                                } else
                                {
                                    ++it;
                                    if (it == end)
                                    {
                                        std::ostringstream problem;
                                        problem << "Flag '" << arg << "' requires an argument but received none";
                                        throw ParseError(problem.str().c_str());
                                    }

                                    if (allowSeparateLongArgument)
                                    {
                                        argbase->ParseArg(*it);
                                    } else
                                    {
                                        std::ostringstream problem;
                                        problem << "Flag '" << arg << "' was passed a separate argument, but these are disallowed";
                                        throw ParseError(problem.str().c_str());
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
                            problem << "Flag could not be matched: " << arg;
                            throw ParseError(problem.str().c_str());
                        }
                        // Check short args
                    } else if (!terminated && chunk.find(shortprefix) == 0 && chunk.size() > shortprefix.size())
                    {
                        const std::string argchunk(chunk.substr(shortprefix.size()));
                        for (auto argit = std::begin(argchunk); argit != std::end(argchunk); ++argit)
                        {
                            const char arg = *argit;

                            Base *base = Match(arg);
                            if (base)
                            {
                                ArgFlagBase *argbase = dynamic_cast<ArgFlagBase *>(base);
                                if (argbase)
                                {
                                    const std::string arg(++argit, std::end(argchunk));
                                    if (!arg.empty())
                                    {
                                        if (allowJoinedShortArgument)
                                        {
                                            argbase->ParseArg(arg);
                                        } else
                                        {
                                            std::ostringstream problem;
                                            problem << "Flag '" << *argit << "' was passed a joined argument, but these are disallowed";
                                            throw ParseError(problem.str().c_str());
                                        }
                                    } else
                                    {
                                        ++it;
                                        if (it == end)
                                        {
                                            std::ostringstream problem;
                                            problem << "Flag '" << *argit << "' requires an argument but received none";
                                            throw ParseError(problem.str().c_str());
                                        }

                                        if (allowSeparateShortArgument)
                                        {
                                            argbase->ParseArg(*it);
                                        } else
                                        {
                                            std::ostringstream problem;
                                            problem << "Flag '" << *argit << "' was passed a separate argument, but these are disallowed";
                                            throw ParseError(problem.str().c_str());
                                        }
                                    }
                                    // Because this argchunk is done regardless
                                    break;
                                }
                            } else
                            {
                                std::ostringstream problem;
                                problem << "Flag could not be matched: '" << arg << "'";
                                throw ParseError(problem.str().c_str());
                            }
                        }
                    } else
                    {
                        PosBase *pos = GetNextPos();
                        if (pos)
                        {
                            pos->ParseArg(chunk);
                        } else
                        {
                            std::ostringstream problem;
                            problem << "Passed in argument, but no positional arguments were ready to receive it: " << chunk;
                            throw ParseError(problem.str().c_str());
                        }
                    }
                }
                if (!Matched())
                {
                    std::ostringstream problem;
                    problem << "Group validation failed somewhere!";
                    throw ValidationError(problem.str().c_str());
                }
            }

            /** Parse all arguments.
             *
             * \param args an iterable of the arguments
             */
            template <typename T>
            void ParseArgs(const T &args)
            {
                ParseArgs(std::begin(args), std::end(args));
            }

            /** Convenience function to parse the CLI from argc and argv
             *
             * Just assigns the program name and vectorizes arguments for passing into ParseArgs()
             */
            void ParseCLI(const int argc, const char * const * const argv)
            {
                if (prog.empty())
                {
                    prog.assign(argv[0]);
                }
                const std::vector<std::string> args(argv + 1, argv + argc);
                ParseArgs(args);
            }
    };

    std::ostream &operator<<(std::ostream &os, const ArgumentParser &parser)
    {
        parser.Help(os);
        return os;
    }

    /** Boolean argument matcher
     */
    class Flag : public FlagBase
    {
        public:
            Flag(Group &group, const std::string &name, const std::string &help, Matcher &&matcher): FlagBase(name, help, std::move(matcher))
            {
                group.Add(*this);
            }

            virtual ~Flag() {}
    };

    /** Help flag class
     *
     * Works like a regular flag, but throws an instance of Help when it is matched
     */
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

    /** A flag class that simply counts the number of times it's matched
     */
    class Counter : public Flag
    {
        public:
            /** The public count variable.  Can be changed at will, but probably shouldn't be.
             */
            int count;

            Counter(Group &group, const std::string &name, const std::string &help, Matcher &&matcher, const int startcount = 0): Flag(group, name, help, std::move(matcher)), count(startcount) {}

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
    };

    /** A default Reader function for argument classes
     *
     * Simply uses a std::istringstream to read into the destination type, and
     * raises a ParseError if there are any characters left.
     */
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

    /** std::string specialization for ArgReader
     *
     * By default, stream extraction into a string splits on white spaces, and
     * it is more efficient to ust copy a string into the destination.
     */
    template <>
    void ArgReader<std::string>(const std::string &name, const std::string &value, std::string &destination)
    {
        destination.assign(value);
    }

    /** An argument-accepting flag class
     * 
     * \tparam T the type to extract the argument as
     * \tparam Reader The function used to read the argument, taking the name, value, and destination reference
     */
    template <typename T, void (*Reader)(const std::string &, const std::string &, T&) = ArgReader<T>>
    class ArgFlag : public ArgFlagBase
    {
        public:
            /** The publicly accessible value member
             *
             * You can change this, but you probably shouldn't.
             */
            T value;

            ArgFlag(Group &group, const std::string &name, const std::string &help, Matcher &&matcher, const T &defaultValue = T()): ArgFlagBase(name, help, std::move(matcher)), value(defaultValue)
            {
                group.Add(*this);
            }

            virtual ~ArgFlag() {}

            virtual void ParseArg(const std::string &value) override
            {
                Reader(name, value, this->value);
            }
    };

    /** An argument-accepting flag class that pushes the found values into a list
     * 
     * \tparam T the type to extract the argument as
     * \tparam List the list type that houses the values
     * \tparam Reader The function used to read the argument, taking the name, value, and destination reference
     */
    template <
        typename T,
        typename List = std::vector<T>,
        void (*Reader)(const std::string &, const std::string &, T&) = ArgReader<T>>
    class ArgFlagList : public ArgFlagBase
    {
        public:
            /** The publicly accessible value member list
             *
             * You can change this, but you probably shouldn't.
             */
            List values;

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
    };

    /** A positional argument class
     *
     * \tparam T the type to extract the argument as
     * \tparam Reader The function used to read the argument, taking the name, value, and destination reference
     */
    template <typename T, void (*Reader)(const std::string &, const std::string &, T&) = ArgReader<T>>
    class PosArg : public PosBase
    {
        public:
            /** The publicly accessible value member
             *
             * You can change this, but you probably shouldn't.
             */
            T value;

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
    };

    /** A positional argument class that pushes the found values into a list
     * 
     * \tparam T the type to extract the argument as
     * \tparam List the list type that houses the values
     * \tparam Reader The function used to read the argument, taking the name, value, and destination reference
     */
    template <
        typename T,
        typename List = std::vector<T>,
        void (*Reader)(const std::string &, const std::string &, T&) = ArgReader<T>>
    class PosArgList : public PosBase
    {
        public:
            /** The publicly accessible value member list
             *
             * You can change this, but you probably shouldn't.
             */
            List values;

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

            virtual std::string Name() const override
            {
                return name + std::string("...");
            }
    };
}
