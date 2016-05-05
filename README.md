# args
A simple, small, flexible, single-header C++11 argument parsing library

This is designed to somewhat replicate the behavior of Python's argparse, but
in C++, with static type checking, and hopefully a lot faster.

UTF-8 support is limited at best.  No normalization is performed, so non-ascii
characters are very best kept out of flags, and combined glyphs are probably
going to mess up help output if you use it.

This program is MIT-licensed, so you can use the header as-is with no
restrictions. I'd appreciate attribution in a README, Man page, or something if
you are feeling generous, but all that's required is that you don't remove the
license and my name from the header of the args.hxx file in source
redistributions (ie. don't pretend that you wrote it).  I do welcome additions
and updates wherever you feel like contributing code.

There is no API documentation here.  The APIs that are most important are the
ArgumentParser and the constructors of the individual types.  The examples
should be, for the most part, enough to use this library, but the API
documentation will come soon enough.

# What does it do

It:

* lets you handle flags, flag+arguments, and positional arguments simply and
	elegently, with the full help of static typechecking.
* allows you to use your own types in a pretty simple way.
* lets you use count flags, and lists of all argument-accepting types.
* Allows full validation of groups of required arguments, though output isn't
	pretty when something fails group validation.  User validation functions are
	accepted.  Groups are fully nestable.
* Generates pretty help for you, with some good tweakable parameters.
* Lets you customize all prefixes and most separators, allowing you to create
	an infinite number of different argument syntaxes
* Lets you parse, by default, any type that has a stream extractor operator for
	it.  If this doesn't work, you can supply a function and parse the string
	yourself if you like.

# What does it not do

There are tons of things this library does not do!

It does not yet:

* Allow you to use a positional argument list before any other positional
	arguments (the last argument list will slurp all subsequent positional
	arguments).  The logic for allowing this would be a lot more code than I'd
	like, and would make static checking much more difficult, requiring us to
	sort std::string arguments and pair them to positional arguments before
	assigning them, rather than what we currently do, which is assiging them as
	we go for better simplicity and speed.

It will not ever:

* Allow you to create subparsers like argparse
* Allow one argument flag to take a specific number of arguments
	(like `--foo first second`).  You can instead split that with a flag list
	(`--foo first --foo second`) or a custom type extraction
	(`--foo first,second`)
* Allow you to intermix multiple different prefix types (eg. `++foo` and
	`--foo` in the same parser), though shortopt and longopt prefixes can be
	different.

# Examples

All the code examples here will be complete code examples, with some output.

## Simple example:

```c++
	args::ArgumentParser parser("This is a test program.", "This goes after the options.");
	args::HelpFlag help(parser, "help", "Display this help menu", args::Matcher({'h'}, {"help"}));
	try
	{
		parser.ParseCLI(argc, argv);
	}
	catch (args::Help)
	{
		std::cout << parser.Help();
		return 0;
	}
	catch (args::ParseError e)
	{
		std::cerr << e.what() << std::endl;
		std::cerr << parser.Help();
		return 1;
	}
	return 0;
}
```

```shell
 % ./test
 % ./test -h
  ./test {OPTIONS} 

    This is a test program. 

  OPTIONS:

      -h, --help         Display this help menu 

    This goes after the options. 
 % 
```

## Boolean flags, special group types, different matcher construction:

```c++
#include <iostream>

#include <args.hxx>

int main(int argc, char **argv)
{
	args::ArgumentParser parser("This is a test program.", "This goes after the options.");
	args::Group group(parser, "This group is all exclusive", args::Group::Validators::Xor);
	args::Flag foo(group, "foo", "The foo flag", args::Matcher({'f'}, {"foo"}));
	args::Flag bar(group, "bar", "The bar flag", args::Matcher({'b'}));
	args::Flag baz(group, "baz", "The baz flag", args::Matcher({"baz"}));
	try
	{
		parser.ParseCLI(argc, argv);
	}
	catch (args::Help)
	{
		std::cout << parser.Help();
		return 0;
	}
	catch (args::ParseError e)
	{
		std::cerr << e.what() << std::endl;
		std::cerr << parser.Help();
		return 1;
	}
	catch (args::ValidationError e)
	{
		std::cerr << e.what() << std::endl;
		std::cerr << parser.Help();
		return 1;
	}
	if (foo) { std::cout << "foo" << std::endl; }
	if (bar) { std::cout << "bar" << std::endl; }
	if (baz) { std::cout << "baz" << std::endl; }
	return 0;
}
```

```shell
 % ./test   
Group validation failed somewhere!
  ./test {OPTIONS} 

    This is a test program. 

  OPTIONS:

      -f, --foo          The foo flag 
      -b                 The bar flag 
      --baz              The baz flag 

    This goes after the options. 
 % ./test -f
foo
 % ./test --foo
foo
 % ./test --foo -f
foo
 % ./test -b      
bar
 % ./test --baz
baz
 % ./test --baz -f
Group validation failed somewhere!
  ./test {OPTIONS} 

    This is a test program. 
...
 % ./test --baz -fb
Group validation failed somewhere!
  ./test {OPTIONS} 
...
 % 
```

## Argument flags, Positional arguments, lists

```c++
```

```shell
% ./test -h
  ./test {OPTIONS} [foo] [numbers...] 

    This is a test program. 

  OPTIONS:

      -h, --help         Display this help menu 
      -i integer         The integer flag 
      -c characters      The character flag 
      foo                The foo position 
      numbers            The numbers position list 
      "--" can be used to terminate flag options and force all following
      arguments to be treated as positional options 

    This goes after the options. 
 % ./test -i 5
i: 5
 % ./test -i 5.2
Argument 'integer' received invalid value type '5.2'
  ./test {OPTIONS} [foo] [numbers...] 
 % ./test -c 1 -c 2 -c 3
c: 1
c: 2
c: 3
 % 
 % ./test 1 2 3 4 5 6 7 8 9
f: 1
n: 2
n: 3
n: 4
n: 5
n: 6
n: 7
n: 8
n: 9
 % ./test 1 2 3 4 5 6 7 8 9 a
Argument 'numbers' received invalid value type 'a'
  ./test {OPTIONS} [foo] [numbers...] 

    This is a test program. 
...
```

# Custom type parsers (here we use std::tuple)

```c++
#include <iostream>
#include <tuple>

#include <args.hxx>

std::istream& operator>>(std::istream& is, std::tuple<int, int>& ints)
{
    is >> std::get<0>(ints);
    is.get();
    is >> std::get<1>(ints);
    return is;
}

void DoublesReader(const std::string &name, const std::string &value, std::tuple<double, double> &destination)
{
    size_t commapos = 0;
    std::get<0>(destination) = std::stod(value, &commapos);
    std::get<1>(destination) = std::stod(std::string(value, commapos + 1));
}

int main(int argc, char **argv)
{
    args::ArgumentParser parser("This is a test program.");
    args::PosArg<std::tuple<int, int>> ints(parser, "INTS", "This takes a pair of integers.");
    args::PosArg<std::tuple<double, double>, DoublesReader> doubles(parser, "DOUBLES", "This takes a pair of doubles.");
    try
    {
        parser.ParseCLI(argc, argv);
    }
    catch (args::Help)
    {
        std::cout << parser.Help();
        return 0;
    }
    catch (args::ParseError e)
    {
        std::cerr << e.what() << std::endl;
        std::cerr << parser.Help();
        return 1;
    }
    if (ints)
    {
        std::cout << "ints found: " << std::get<0>(ints.value) << " and " << std::get<1>(ints.value) << std::endl;
    }
    if (doubles)
    {
        std::cout << "doubles found: " << std::get<0>(doubles.value) << " and " << std::get<1>(doubles.value) << std::endl;
    }
    return 0;
}
```

```shell
 % ./test -h
Argument could not be matched: 'h'
  ./test [INTS] [DOUBLES] 

    This is a test program. 

  OPTIONS:

      INTS               This takes a pair of integers. 
      DOUBLES            This takes a pair of doubles. 

 % ./test 5
ints found: 5 and 0
 % ./test 5,8
ints found: 5 and 8
 % ./test 5,8 2.4,8
ints found: 5 and 8
doubles found: 2.4 and 8
 % ./test 5,8 2.4, 
terminate called after throwing an instance of 'std::invalid_argument'
  what():  stod
zsh: abort      ./test 5,8 2.4,
 % ./test 5,8 2.4 
terminate called after throwing an instance of 'std::out_of_range'
  what():  basic_string::basic_string: __pos (which is 4) > this->size() (which is 3)
zsh: abort      ./test 5,8 2.4
 % ./test 5,8 2.4-7
ints found: 5 and 8
doubles found: 2.4 and 7
 % ./test 5,8 2.4,-7
ints found: 5 and 8
doubles found: 2.4 and -7
```

As you can see, with your own types, validation can get a little weird.  Make
sure to check and throw a parsing error (or whatever error you want to catch)
if you can't fully deduce your type.  The built-in validator will only throw if
there are unextracted characters left in the stream.
