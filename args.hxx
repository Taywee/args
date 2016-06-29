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

/** \file args.hxx
 * \brief this single-header lets you use all of the args functionality
 *
 * The important stuff is done inside the args namespace
 */

#include <algorithm>
#include <exception>
#include <functional>
#include <sstream>
#include <string>
#include <tuple>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <type_traits>

/** \namespace args
 * \brief contains all the functionality of the args library
 */
namespace args
{
    /** Getter to grab the value from the argument type.
     *
     * If the Get() function of the type returns a reference, so does this, and
     * the value will be modifiable.
     */
    template <typename Option>
    auto get(Option &option) -> decltype(option.Get())
    {
        return option.Get();
    }

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

    /** Base error class
     */
    class Error : public std::runtime_error
    {
        public:
            Error(const std::string &problem) : std::runtime_error(problem) {}
            virtual ~Error() {};
    };

    /** Errors that occur during regular parsing
     */
    class ParseError : public Error
    {
        public:
            ParseError(const std::string &problem) : Error(problem) {}
            virtual ~ParseError() {};
    };

    /** Errors that are detected from group validation after parsing finishes
     */
    class ValidationError : public Error
    {
        public:
            ValidationError(const std::string &problem) : Error(problem) {}
            virtual ~ValidationError() {};
    };

    /** Errors in map lookups
     */
    class MapError : public ParseError
    {
        public:
            MapError(const std::string &problem) : ParseError(problem) {}
            virtual ~MapError() {};
    };

    /** Error that occurs when a singular flag is specified multiple times
     */
    class ExtraError : public ParseError
    {
        public:
            ExtraError(const std::string &problem) : ParseError(problem) {}
            virtual ~ExtraError() {};
    };

    /** An exception that indicates that the user has requested help
     */
    class Help : public Error
    {
        public:
            Help(const std::string &flag) : Error(flag) {}
            virtual ~Help() {};
    };

    /** A simple unified option type for unified initializer lists for the Matcher class.
     */
    struct EitherFlag
    {
        const bool isShort;
        const char shortFlag;
        const std::string longFlag;
        EitherFlag(const std::string &flag) : isShort(false), shortFlag(), longFlag(flag) {}
        EitherFlag(const char *flag) : isShort(false), shortFlag(), longFlag(flag) {}
        EitherFlag(const char flag) : isShort(true), shortFlag(flag), longFlag() {}

        /** Get just the long flags from an initializer list of EitherFlags
         */
        static std::unordered_set<std::string> GetLong(std::initializer_list<EitherFlag> flags)
        {
            std::unordered_set<std::string>  longFlags;
            for (const EitherFlag &flag: flags)
            {
                if (!flag.isShort)
                {
                    longFlags.insert(flag.longFlag);
                }
            }
            return longFlags;
        }

        /** Get just the short flags from an initializer list of EitherFlags
         */
        static std::unordered_set<char> GetShort(std::initializer_list<EitherFlag> flags)
        {
            std::unordered_set<char>  shortFlags;
            for (const EitherFlag &flag: flags)
            {
                if (flag.isShort)
                {
                    shortFlags.insert(flag.shortFlag);
                }
            }
            return shortFlags;
        }
    };



    /** A class of "matchers", specifying short and flags that can possibly be
     * matched.
     *
     * This is supposed to be constructed and then passed in, not used directly
     * from user code.
     */
    class Matcher
    {
        private:
            const std::unordered_set<char> shortFlags;
            const std::unordered_set<std::string> longFlags;

        public:
            /** Specify short and long flags separately as iterators
             *
             * ex: `args::Matcher(shortFlags.begin(), shortFlags.end(), longFlags.begin(), longFlags.end())`
             */
            template <typename ShortIt, typename LongIt>
            Matcher(ShortIt shortFlagsStart, ShortIt shortFlagsEnd, LongIt longFlagsStart, LongIt longFlagsEnd) :
                shortFlags(shortFlagsStart, shortFlagsEnd),
                longFlags(longFlagsStart, longFlagsEnd)
            {}

            /** Specify short and long flags separately as iterables
             *
             * ex: `args::Matcher(shortFlags, longFlags)`
             */
            template <typename Short, typename Long>
            Matcher(Short &&shortIn, Long &&longIn) :
                shortFlags(std::begin(shortIn), std::end(shortIn)), longFlags(std::begin(longIn), std::end(longIn))
            {}

            /** Specify a mixed single initializer-list of both short and long flags
             *
             * This is the fancy one.  It takes a single initializer list of
             * any number of any mixed kinds of flags.  Chars are
             * automatically interpreted as short flags, and strings are
             * automatically interpreted as long flags:
             *
             *     args::Matcher{'a'}
             *     args::Matcher{"foo"}
             *     args::Matcher{'h', "help"}
             *     args::Matcher{"foo", 'f', 'F', "FoO"}
             */
            Matcher(std::initializer_list<EitherFlag> in) :
                shortFlags(EitherFlag::GetShort(in)), longFlags(EitherFlag::GetLong(in)) {}

            Matcher(Matcher &&other) : shortFlags(std::move(other.shortFlags)), longFlags(std::move(other.longFlags))
            {}

            ~Matcher() {}

            /** (INTERNAL) Check if there is a match of a short flag
             */
            bool Match(const char flag) const
            {
                return shortFlags.find(flag) != shortFlags.end();
            }

            /** (INTERNAL) Check if there is a match of a long flag
             */
            bool Match(const std::string &flag) const
            {
                return longFlags.find(flag) != longFlags.end();
            }

            /** (INTERNAL) Get all flag strings as a vector, with the prefixes embedded
             */
            std::vector<std::string> GetFlagStrings(const std::string &shortPrefix, const std::string &longPrefix) const
            {
                std::vector<std::string> flagStrings;
                flagStrings.reserve(shortFlags.size() + longFlags.size());
                for (const char flag: shortFlags)
                {
                    flagStrings.emplace_back(shortPrefix + std::string(1, flag));
                }
                for (const std::string &flag: longFlags)
                {
                    flagStrings.emplace_back(longPrefix + flag);
                }
                return flagStrings;
            }

            /** (INTERNAL) Get all flag strings as a vector, with the prefixes and names embedded
             */
            std::vector<std::string> GetFlagStrings(const std::string &shortPrefix, const std::string &longPrefix, const std::string &name, const std::string &shortSeparator, const std::string longSeparator) const
            {
                const std::string bracedname(std::string("[") + name + "]");
                std::vector<std::string> flagStrings;
                flagStrings.reserve(shortFlags.size() + longFlags.size());
                for (const char flag: shortFlags)
                {
                    flagStrings.emplace_back(shortPrefix + std::string(1, flag) + shortSeparator + bracedname);
                }
                for (const std::string &flag: longFlags)
                {
                    flagStrings.emplace_back(longPrefix + flag + longSeparator + bracedname);
                }
                return flagStrings;
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
            bool kickout;

        public:
            NamedBase(const std::string &name, const std::string &help) : Base(help), name(name), kickout(false) {}
            virtual ~NamedBase() {}

            virtual std::tuple<std::string, std::string> GetDescription(const std::string &shortPrefix, const std::string &longPrefi, const std::string &shortSeparator, const std::string &longSeparator) const override
            {
                std::tuple<std::string, std::string> description;
                std::get<0>(description) = Name();
                std::get<1>(description) = help;
                return description;
            }
            virtual std::string Name() const
            {
                return name;
            }

            /// Sets a kick-out value for building subparsers
            void KickOut(bool kickout) noexcept
            {
                this->kickout = kickout;
            }

            /// Gets the kick-out value for building subparsers
            bool KickOut() const noexcept
            {
                return kickout;
            }
    };

    /** Base class for all flag options
     */
    class FlagBase : public NamedBase
    {
        private:
            const bool extraError;

        protected:
            const Matcher matcher;

        public:
            FlagBase(const std::string &name, const std::string &help, Matcher &&matcher, const bool extraError = false) : NamedBase(name, help), extraError(extraError), matcher(std::move(matcher)) {}

            virtual ~FlagBase() {}

            virtual FlagBase *Match(const std::string &flag)
            {
                if (matcher.Match(flag))
                {
                    if (extraError && matched)
                    {
                        std::ostringstream problem;
                        problem << "Flag '" << flag << "' was passed multiple times, but is only allowed to be passed once";
                        throw ExtraError(problem.str());
                    }
                    matched = true;
                    return this;
                }
                return nullptr;
            }

            virtual FlagBase *Match(const char flag)
            {
                if (matcher.Match(flag))
                {
                    if (extraError && matched)
                    {
                        std::ostringstream problem;
                        problem << "Flag '" << flag << "' was passed multiple times, but is only allowed to be passed once";
                        throw ExtraError(problem.str());
                    }
                    matched = true;
                    return this;
                }
                return nullptr;
            }

            virtual std::tuple<std::string, std::string> GetDescription(const std::string &shortPrefix, const std::string &longPrefix, const std::string &shortSeparator, const std::string &longSeparator) const override
            {
                std::tuple<std::string, std::string> description;
                const std::vector<std::string> flagStrings(matcher.GetFlagStrings(shortPrefix, longPrefix));
                std::ostringstream flagstream;
                for (auto it = std::begin(flagStrings); it != std::end(flagStrings); ++it)
                {
                    if (it != std::begin(flagStrings))
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

    /** Base class for value-accepting flag options
     */
    class ValueFlagBase : public FlagBase
    {
        public:
            ValueFlagBase(const std::string &name, const std::string &help, Matcher &&matcher, const bool extraError = false) : FlagBase(name, help, std::move(matcher), extraError) {}
            virtual ~ValueFlagBase() {}
            virtual void ParseValue(const std::string &value) = 0;

            virtual std::tuple<std::string, std::string> GetDescription(const std::string &shortPrefix, const std::string &longPrefix, const std::string &shortSeparator, const std::string &longSeparator) const override
            {
                std::tuple<std::string, std::string> description;
                const std::vector<std::string> flagStrings(matcher.GetFlagStrings(shortPrefix, longPrefix, Name(), shortSeparator, longSeparator));
                std::ostringstream flagstream;
                for (auto it = std::begin(flagStrings); it != std::end(flagStrings); ++it)
                {
                    if (it != std::begin(flagStrings))
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

    /** Base class for positional options
     */
    class PositionalBase : public NamedBase
    {
        protected:
            bool ready;

        public:
            PositionalBase(const std::string &name, const std::string &help) : NamedBase(name, help), ready(true) {}
            virtual ~PositionalBase() {}

            bool Ready()
            {
                return ready;
            }

            virtual void ParseValue(const std::string &value) = 0;
    };

    /** Class for all kinds of validating groups, including ArgumentParser
     */
    class Group : public Base
    {
        private:
            std::vector<Base*> children;
            std::function<bool(const Group &)> validator;

        public:
            Group(const std::string &help = std::string(), const std::function<bool(const Group &)> &validator = Validators::DontCare) : Base(help), validator(validator) {}
            Group(Group &group, const std::string &help = std::string(), const std::function<bool(const Group &)> &validator = Validators::DontCare) : Base(help), validator(validator)
            {
                group.Add(*this);
            }
            virtual ~Group() {}

            /** Return the first FlagBase that matches flag, or nullptr
             *
             * \param flag The flag with prefixes stripped
             * \return the first matching FlagBase pointer, or nullptr if there is no match
             */
            FlagBase *Match(const std::string &flag)
            {
                for (Base *child: children)
                {
                    if (FlagBase *flagBase = dynamic_cast<FlagBase *>(child))
                    {
                        if (FlagBase *match = flagBase->Match(flag))
                        {
                            return match;
                        }
                    } else if (Group *group = dynamic_cast<Group *>(child))
                    {
                        if (FlagBase *match = group->Match(flag))
                        {
                            return match;
                        }
                    }
                }
                return nullptr;
            }

            /** Return the first FlagBase that matches flag, or nullptr
             *
             * \param flag The flag with prefixes stripped
             * \return the first matching FlagBase pointer, or nullptr if there is no match
             */
            FlagBase *Match(const char flag)
            {
                for (Base *child: children)
                {
                    if (FlagBase *flagBase = dynamic_cast<FlagBase *>(child))
                    {
                        if (FlagBase *match = flagBase->Match(flag))
                        {
                            return match;
                        }
                    } else if (Group *group = dynamic_cast<Group *>(child))
                    {
                        if (FlagBase *match = group->Match(flag))
                        {
                            return match;
                        }
                    }
                }
                return nullptr;
            }

            /** Get the next ready positional, or nullptr if there is none
             *
             * \return the first ready PositionalBase pointer, or nullptr if there is no match
             */
            PositionalBase *GetNextPositional()
            {
                for (Base *child: children)
                {
                    PositionalBase *next = dynamic_cast<PositionalBase *>(child);
                    Group *group = dynamic_cast<Group *>(child);
                    if (group)
                    {
                        next = group->GetNextPositional();
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
                    if (dynamic_cast<FlagBase *>(child))
                    {
                        return true;
                    }
                    if (Group *group = dynamic_cast<Group *>(child))
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

            /** Get validation
             */
            bool Get() const
            {
                return Matched();
            }

            /** Get all the child descriptions for help generation
             */
            std::vector<std::tuple<std::string, std::string, unsigned int>> GetChildDescriptions(const std::string &shortPrefix, const std::string &longPrefix, const std::string &shortSeparator, const std::string &longSeparator, const unsigned int indent = 0) const
            {
                std::vector<std::tuple<std::string, std::string, unsigned int>> descriptions;
                for (const auto &child: children)
                {
                    if (const Group *group = dynamic_cast<Group *>(child))
                    {
                        // Push that group description on the back if not empty
                        unsigned char addindent = 0;
                        if (!group->help.empty())
                        {
                            descriptions.emplace_back(group->help, "", indent);
                            addindent = 1;
                        }
                        std::vector<std::tuple<std::string, std::string, unsigned int>> groupDescriptions(group->GetChildDescriptions(shortPrefix, longPrefix, shortSeparator, longSeparator, indent + addindent));
                        descriptions.insert(
                            std::end(descriptions),
                            std::make_move_iterator(std::begin(groupDescriptions)),
                            std::make_move_iterator(std::end(groupDescriptions)));
                    } else if (const NamedBase *named = dynamic_cast<NamedBase *>(child))
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
                    if (const Group *group = dynamic_cast<Group *>(child))
                    {
                        std::vector<std::string> groupNames(group->GetPosNames());
                        names.insert(
                            std::end(names),
                            std::make_move_iterator(std::begin(groupNames)),
                            std::make_move_iterator(std::end(groupNames)));
                    } else if (const PositionalBase *pos = dynamic_cast<PositionalBase *>(child))
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
                        if (const Group *group = dynamic_cast<Group *>(child))
                        {
                            if (!group->Matched())
                            {
                                return false;
                            }
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
            std::string proglinePostfix;
            std::string description;
            std::string epilog;

            std::string longprefix;
            std::string shortprefix;

            std::string longseparator;

            std::string terminator;

            bool allowJoinedShortValue;
            bool allowJoinedLongValue;
            bool allowSeparateShortValue;
            bool allowSeparateLongValue;

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

                /** Show the terminator when both options and positional parameters are present
                 */
                bool showTerminator = true;

                /** Show the {OPTIONS} on the prog line when this is true
                 */
                bool showProglineOptions = true;

                /** Show the positionals on the prog line when this is true
                 */
                bool showProglinePositionals = true;
            } helpParams;
            ArgumentParser(const std::string &description, const std::string &epilog = std::string()) :
                Group("", Group::Validators::AllChildGroups),
                description(description),
                epilog(epilog),
                longprefix("--"),
                shortprefix("-"),
                longseparator("="),
                terminator("--"),
                allowJoinedShortValue(true),
                allowJoinedLongValue(true),
                allowSeparateShortValue(true),
                allowSeparateLongValue(true) {}

            /** The program name for help generation
             */
            const std::string &Prog() const
            { return prog; }
            /** The program name for help generation
             */
            void Prog(const std::string &prog)
            { this->prog = prog; }

            /** The description that appears on the prog line after options
             */
            const std::string &ProglinePostfix() const
            { return proglinePostfix; }
            /** The description that appears on the prog line after options
             */
            void ProglinePostfix(const std::string &proglinePostfix)
            { this->proglinePostfix = proglinePostfix; }

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

            /** The prefix for long flags
             */
            const std::string &LongPrefix() const
            { return longprefix; }
            /** The prefix for long flags
             */
            void LongPrefix(const std::string &longprefix)
            { this->longprefix = longprefix; }

            /** The prefix for short flags
             */
            const std::string &ShortPrefix() const
            { return shortprefix; }
            /** The prefix for short flags
             */
            void ShortPrefix(const std::string &shortprefix)
            { this->shortprefix = shortprefix; }

            /** The separator for long flags
             */
            const std::string &LongSeparator() const
            { return longseparator; }
            /** The separator for long flags
             */
            void LongSeparator(const std::string &longseparator)
            {
                if (longseparator.empty())
                {
                    throw std::runtime_error("longseparator can not be set to empty");
                }
                this->longseparator = longseparator;
            }

            /** The terminator that forcibly separates flags from positionals
             */
            const std::string &Terminator() const
            { return terminator; }
            /** The terminator that forcibly separates flags from positionals
             */
            void Terminator(const std::string &terminator)
            { this->terminator = terminator; }

            /** Get the current argument separation parameters.
             *
             * See SetArgumentSeparations for details on what each one means.
             */
            void GetArgumentSeparations(
                bool &allowJoinedShortValue,
                bool &allowJoinedLongValue,
                bool &allowSeparateShortValue,
                bool &allowSeparateLongValue) const
            {
                allowJoinedShortValue = this->allowJoinedShortValue;
                allowJoinedLongValue = this->allowJoinedLongValue;
                allowSeparateShortValue = this->allowSeparateShortValue;
                allowSeparateLongValue = this->allowSeparateLongValue;
            }

            /** Change allowed option separation.
             *
             * \param allowJoinedShortValue Allow a short flag that accepts an argument to be passed its argument immediately next to it (ie. in the same argv field)
             * \param allowJoinedLongValue Allow a long flag that accepts an argument to be passed its argument separated by the longseparator (ie. in the same argv field)
             * \param allowSeparateShortValue Allow a short flag that accepts an argument to be passed its argument separated by whitespace (ie. in the next argv field)
             * \param allowSeparateLongValue Allow a long flag that accepts an argument to be passed its argument separated by whitespace (ie. in the next argv field)
             */
            void SetArgumentSeparations(
                const bool allowJoinedShortValue,
                const bool allowJoinedLongValue,
                const bool allowSeparateShortValue,
                const bool allowSeparateLongValue)
            {
                this->allowJoinedShortValue = allowJoinedShortValue;
                this->allowJoinedLongValue = allowJoinedLongValue;
                this->allowSeparateShortValue = allowSeparateShortValue;
                this->allowSeparateLongValue = allowSeparateLongValue;
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
                    if (helpParams.showProglineOptions)
                    {
                        prognameline << " {OPTIONS}";
                    }
                }
                for (const std::string &posname: GetPosNames())
                {
                    hasarguments = true;
                    if (helpParams.showProglinePositionals)
                    {
                        prognameline << " [" << posname << ']';
                    }
                }
                if (!proglinePostfix.empty())
                {
                    prognameline << ' ' << proglinePostfix;
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
                for (const auto &description: GetChildDescriptions(shortprefix, longprefix, allowJoinedShortValue ? "" : " ", allowJoinedLongValue ? longseparator : " "))
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
                if (hasoptions && hasarguments && helpParams.showTerminator)
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
             * \return the iterator after the last parsed value.  Only useful for kick-out
             */
            template <typename It>
            It ParseArgs(It begin, It end)
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

                        if (FlagBase *base = Match(arg))
                        {
                            if (ValueFlagBase *argbase = dynamic_cast<ValueFlagBase *>(base))
                            {
                                if (separator != argchunk.npos)
                                {
                                    if (allowJoinedLongValue)
                                    {
                                        argbase->ParseValue(argchunk.substr(separator + longseparator.size()));
                                    } else
                                    {
                                        std::ostringstream problem;
                                        problem << "Flag '" << arg << "' was passed a joined argument, but these are disallowed";
                                        throw ParseError(problem.str());
                                    }
                                } else
                                {
                                    ++it;
                                    if (it == end)
                                    {
                                        std::ostringstream problem;
                                        problem << "Flag '" << arg << "' requires an argument but received none";
                                        throw ParseError(problem.str());
                                    }

                                    if (allowSeparateLongValue)
                                    {
                                        argbase->ParseValue(*it);
                                    } else
                                    {
                                        std::ostringstream problem;
                                        problem << "Flag '" << arg << "' was passed a separate argument, but these are disallowed";
                                        throw ParseError(problem.str());
                                    }
                                }
                            } else if (separator != argchunk.npos)
                            {
                                std::ostringstream problem;
                                problem << "Passed an argument into a non-argument flag: " << chunk;
                                throw ParseError(problem.str());
                            }

                            if (base->KickOut())
                            {
                                return ++it;
                            }
                        } else
                        {
                            std::ostringstream problem;
                            problem << "Flag could not be matched: " << arg;
                            throw ParseError(problem.str());
                        }
                        // Check short args
                    } else if (!terminated && chunk.find(shortprefix) == 0 && chunk.size() > shortprefix.size())
                    {
                        const std::string argchunk(chunk.substr(shortprefix.size()));
                        for (auto argit = std::begin(argchunk); argit != std::end(argchunk); ++argit)
                        {
                            const char arg = *argit;

                            if (FlagBase *base = Match(arg))
                            {
                                if (ValueFlagBase *argbase = dynamic_cast<ValueFlagBase *>(base))
                                {
                                    const std::string value(++argit, std::end(argchunk));
                                    if (!value.empty())
                                    {
                                        if (allowJoinedShortValue)
                                        {
                                            argbase->ParseValue(value);
                                        } else
                                        {
                                            std::ostringstream problem;
                                            problem << "Flag '" << arg << "' was passed a joined argument, but these are disallowed";
                                            throw ParseError(problem.str());
                                        }
                                    } else
                                    {
                                        ++it;
                                        if (it == end)
                                        {
                                            std::ostringstream problem;
                                            problem << "Flag '" << arg << "' requires an argument but received none";
                                            throw ParseError(problem.str());
                                        }

                                        if (allowSeparateShortValue)
                                        {
                                            argbase->ParseValue(*it);
                                        } else
                                        {
                                            std::ostringstream problem;
                                            problem << "Flag '" << arg << "' was passed a separate argument, but these are disallowed";
                                            throw ParseError(problem.str());
                                        }
                                    }
                                    // Because this argchunk is done regardless
                                    break;
                                }

                                if (base->KickOut())
                                {
                                    return ++it;
                                }
                            } else
                            {
                                std::ostringstream problem;
                                problem << "Flag could not be matched: '" << arg << "'";
                                throw ParseError(problem.str());
                            }
                        }
                    } else
                    {
                        PositionalBase *pos = GetNextPositional();
                        if (pos)
                        {
                            pos->ParseValue(chunk);

                            if (pos->KickOut())
                            {
                                return ++it;
                            }
                        } else
                        {
                            std::ostringstream problem;
                            problem << "Passed in argument, but no positional arguments were ready to receive it: " << chunk;
                            throw ParseError(problem.str());
                        }
                    }
                }
                if (!Matched())
                {
                    std::ostringstream problem;
                    problem << "Group validation failed somewhere!";
                    throw ValidationError(problem.str());
                }
                return end;
            }

            /** Parse all arguments.
             *
             * \param args an iterable of the arguments
             * \return the iterator after the last parsed value.  Only useful for kick-out
             */
            template <typename T>
            auto ParseArgs(const T &args) -> decltype(std::begin(args))
            {
                return ParseArgs(std::begin(args), std::end(args));
            }

            /** Convenience function to parse the CLI from argc and argv
             *
             * Just assigns the program name and vectorizes arguments for passing into ParseArgs()
             *
             * \return whether or not all arguments were parsed.  This works for detecting kick-out, but is generally useless as it can't do anything with it.
             */
            bool ParseCLI(const int argc, const char * const * const argv)
            {
                if (prog.empty())
                {
                    prog.assign(argv[0]);
                }
                const std::vector<std::string> args(argv + 1, argv + argc);
                return ParseArgs(args) == std::end(args);
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
            Flag(Group &group, const std::string &name, const std::string &help, Matcher &&matcher, const bool extraError = false): FlagBase(name, help, std::move(matcher), extraError)
            {
                group.Add(*this);
            }

            virtual ~Flag() {}

            /** Get whether this was matched
             */
            bool Get() const
            {
                return Matched();
            }
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

            /** Get whether this was matched
             */
            bool Get() const noexcept
            {
                return Matched();
            }
    };

    /** A flag class that simply counts the number of times it's matched
     */
    class CounterFlag : public Flag
    {
        private:
            int count;

        public:
            CounterFlag(Group &group, const std::string &name, const std::string &help, Matcher &&matcher, const int startcount = 0): Flag(group, name, help, std::move(matcher)), count(startcount) {}

            virtual ~CounterFlag() {}

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

            /** Get the count
             */
            int &Get() noexcept
            {
                return count;
            }
    };

    /** A default Reader function for argument classes
     *
     * Simply uses a std::istringstream to read into the destination type, and
     * raises a ParseError if there are any characters left.
     */
    template <typename T>
    void ValueReader(const std::string &name, const std::string &value, T &destination)
    {
        std::istringstream ss(value);
        ss >> destination;

        if (ss.rdbuf()->in_avail() > 0)
        {
            std::ostringstream problem;
            problem << "Argument '" << name << "' received invalid value type '" << value << "'";
            throw ParseError(problem.str());
        }
    }

    /** std::string specialization for ValueReader
     *
     * By default, stream extraction into a string splits on white spaces, and
     * it is more efficient to ust copy a string into the destination.
     */
    template <>
    void ValueReader<std::string>(const std::string &name, const std::string &value, std::string &destination)
    {
        destination.assign(value);
    }

    /** An argument-accepting flag class
     * 
     * \tparam T the type to extract the argument as
     * \tparam Reader The function used to read the argument, taking the name, value, and destination reference
     */
    template <typename T, void (*Reader)(const std::string &, const std::string &, T&) = ValueReader<T>>
    class ValueFlag : public ValueFlagBase
    {
        private:
            T value;

        public:

            ValueFlag(Group &group, const std::string &name, const std::string &help, Matcher &&matcher, const T &defaultValue = T(), const bool extraError = false): ValueFlagBase(name, help, std::move(matcher), extraError), value(defaultValue)
            {
                group.Add(*this);
            }

            virtual ~ValueFlag() {}

            virtual void ParseValue(const std::string &value) override
            {
                Reader(name, value, this->value);
            }

            /** Get the value
             */
            T &Get() noexcept
            {
                return value;
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
        void (*Reader)(const std::string &, const std::string &, T&) = ValueReader<T>>
    class ValueFlagList : public ValueFlagBase
    {
        private:
            List values;

        public:

            ValueFlagList(Group &group, const std::string &name, const std::string &help, Matcher &&matcher, const List &defaultValues = List()): ValueFlagBase(name, help, std::move(matcher)), values(defaultValues)
            {
                group.Add(*this);
            }

            virtual ~ValueFlagList() {}

            virtual void ParseValue(const std::string &value) override
            {
                T v;
                Reader(name, value, v);
                values.insert(std::end(values), v);
            }

            /** Get the values
             */
            List &Get() noexcept
            {
                return values;
            }

            virtual std::string Name() const override
            {
                return name + std::string("...");
            }
    };

    /** A mapping value flag class
     * 
     * \tparam K the type to extract the argument as
     * \tparam T the type to store the result as
     * \tparam Reader The function used to read the argument into the key type, taking the name, value, and destination reference
     * \tparam Map The Map type.  Should operate like std::map or std::unordered_map
     */
    template <typename K, typename T, void (*Reader)(const std::string &, const std::string &, K&) = ValueReader<K>, typename Map = std::unordered_map<K, T>>
    class MapFlag : public ValueFlagBase
    {
        private:
            const Map map;
            T value;

        public:

            MapFlag(Group &group, const std::string &name, const std::string &help, Matcher &&matcher, const Map &map, const T &defaultValue = T(), const bool extraError = false): ValueFlagBase(name, help, std::move(matcher), extraError), map(map), value(defaultValue)
            {
                group.Add(*this);
            }

            virtual ~MapFlag() {}

            virtual void ParseValue(const std::string &value) override
            {
                K key;
                Reader(name, value, key);
                auto it = map.find(key);
                if (it == std::end(map))
                {
                    std::ostringstream problem;
                    problem << "Could not find key '" << key << "' in map for arg '" << name << "'";
                    throw MapError(problem.str());
                } else
                {
                    this->value = it->second;
                }
            }

            /** Get the value
             */
            T &Get() noexcept
            {
                return value;
            }
    };

    /** A mapping value flag list class
     * 
     * \tparam K the type to extract the argument as
     * \tparam T the type to store the result as
     * \tparam List the list type that houses the values
     * \tparam Reader The function used to read the argument into the key type, taking the name, value, and destination reference
     * \tparam Map The Map type.  Should operate like std::map or std::unordered_map
     */
    template <typename K, typename T, typename List = std::vector<T>, void (*Reader)(const std::string &, const std::string &, K&) = ValueReader<K>, typename Map = std::unordered_map<K, T>>
    class MapFlagList : public ValueFlagBase
    {
        private:
            const Map map;
            List values;

        public:

            MapFlagList(Group &group, const std::string &name, const std::string &help, Matcher &&matcher, const Map &map, const List &defaultValues = List()): ValueFlagBase(name, help, std::move(matcher)), map(map), values(defaultValues)
            {
                group.Add(*this);
            }

            virtual ~MapFlagList() {}

            virtual void ParseValue(const std::string &value) override
            {
                K key;
                Reader(name, value, key);
                auto it = map.find(key);
                if (it == std::end(map))
                {
                    std::ostringstream problem;
                    problem << "Could not find key '" << key << "' in map for arg '" << name << "'";
                    throw MapError(problem.str());
                } else
                {
                    this->values.emplace_back(it->second);
                }
            }

            /** Get the value
             */
            List &Get() noexcept
            {
                return values;
            }

            virtual std::string Name() const override
            {
                return name + std::string("...");
            }
    };

    /** A positional argument class
     *
     * \tparam T the type to extract the argument as
     * \tparam Reader The function used to read the argument, taking the name, value, and destination reference
     */
    template <typename T, void (*Reader)(const std::string &, const std::string &, T&) = ValueReader<T>>
    class Positional : public PositionalBase
    {
        private:
            T value;
        public:
            Positional(Group &group, const std::string &name, const std::string &help, const T &defaultValue = T()): PositionalBase(name, help), value(defaultValue)
            {
                group.Add(*this);
            }

            virtual ~Positional() {}

            virtual void ParseValue(const std::string &value) override
            {
                Reader(name, value, this->value);
                ready = false;
                matched = true;
            }

            /** Get the value
             */
            T &Get() noexcept
            {
                return value;
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
        void (*Reader)(const std::string &, const std::string &, T&) = ValueReader<T>>
    class PositionalList : public PositionalBase
    {
        private:
            List values;

        public:
            PositionalList(Group &group, const std::string &name, const std::string &help, const List &defaultValues = List()): PositionalBase(name, help), values(defaultValues)
            {
                group.Add(*this);
            }

            virtual ~PositionalList() {}

            virtual void ParseValue(const std::string &value) override
            {
                T v;
                Reader(name, value, v);
                values.insert(std::end(values), v);
                matched = true;
            }

            virtual std::string Name() const override
            {
                return name + std::string("...");
            }

            /** Get the values
             */
            List &Get() noexcept
            {
                return values;
            }
    };

    /** A positional argument mapping class
     * 
     * \tparam K the type to extract the argument as
     * \tparam T the type to store the result as
     * \tparam Reader The function used to read the argument into the key type, taking the name, value, and destination reference
     * \tparam Map The Map type.  Should operate like std::map or std::unordered_map
     */
    template <typename K, typename T, void (*Reader)(const std::string &, const std::string &, K&) = ValueReader<K>, typename Map = std::unordered_map<K, T>>
    class MapPositional : public PositionalBase
    {
        private:
            const Map map;
            T value;

        public:

            MapPositional(Group &group, const std::string &name, const std::string &help, const Map &map, const T &defaultValue = T()): PositionalBase(name, help), map(map), value(defaultValue)
            {
                group.Add(*this);
            }

            virtual ~MapPositional() {}

            virtual void ParseValue(const std::string &value) override
            {
                K key;
                Reader(name, value, key);
                auto it = map.find(key);
                if (it == std::end(map))
                {
                    std::ostringstream problem;
                    problem << "Could not find key '" << key << "' in map for arg '" << name << "'";
                    throw MapError(problem.str());
                } else
                {
                    this->value = it->second;
                    ready = false;
                    matched = true;
                }
            }

            /** Get the value
             */
            T &Get() noexcept
            {
                return value;
            }
    };

    /** A positional argument mapping list class
     * 
     * \tparam K the type to extract the argument as
     * \tparam T the type to store the result as
     * \tparam List the list type that houses the values
     * \tparam Reader The function used to read the argument into the key type, taking the name, value, and destination reference
     * \tparam Map The Map type.  Should operate like std::map or std::unordered_map
     */
    template <typename K, typename T, typename List = std::vector<T>, void (*Reader)(const std::string &, const std::string &, K&) = ValueReader<K>, typename Map = std::unordered_map<K, T>>
    class MapPositionalList : public PositionalBase
    {
        private:
            const Map map;
            List values;

        public:

            MapPositionalList(Group &group, const std::string &name, const std::string &help, const Map &map, const List &defaultValues = List()): PositionalBase(name, help), map(map), values(defaultValues)
            {
                group.Add(*this);
            }

            virtual ~MapPositionalList() {}

            virtual void ParseValue(const std::string &value) override
            {
                K key;
                Reader(name, value, key);
                auto it = map.find(key);
                if (it == std::end(map))
                {
                    std::ostringstream problem;
                    problem << "Could not find key '" << key << "' in map for arg '" << name << "'";
                    throw MapError(problem.str());
                } else
                {
                    this->values.emplace_back(it->second);
                    matched = true;
                }
            }

            /** Get the value
             */
            List &Get() noexcept
            {
                return values;
            }

            virtual std::string Name() const override
            {
                return name + std::string("...");
            }
    };
}
