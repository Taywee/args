// PoC for ARGS_NOEXCEPT iterator OOB and dangling-iterator bugs fixed
// in commit a1407f8. Exercises both sites:
//   Site 1: ParseArgsValues completion path sets `it = end`, caller does ++it
//           on an already-end iterator -> OOB increment + OOB read.
//   Site 2: ParseLong returns an iterator into a local `curArgs` vector that
//           is destroyed when the function returns -> dangling iterator
//           returned to ParseCLI and compared with == against outer end.
//
// Build:
//   clang++ -std=c++11 -O0 -g -fsanitize=address,undefined \
//       -DARGS_HXX_PATH='"args_buggy.hxx"'  poc_noexcept_iter_oob.cxx -o poc_buggy
//   clang++ -std=c++11 -O0 -g -fsanitize=address,undefined \
//       -DARGS_HXX_PATH='"args_fixed.hxx"'  poc_noexcept_iter_oob.cxx -o poc_fixed
//
// Run:
//   ./poc_buggy   # expect ASan heap-buffer-overflow + UBSan dangling compare
//   ./poc_fixed   # expect clean exit

#define ARGS_NOEXCEPT

#ifndef ARGS_HXX_PATH
#define ARGS_HXX_PATH "args.hxx"
#endif

#include ARGS_HXX_PATH

#include <cstdio>
#include <vector>
#include <string>

int main()
{
    // -------- Site 1: ARGS_NOEXCEPT completion on a flag value position --------
    // cword=2 lands on the empty value position of "-m". On the buggy build
    // ParseArgsValues sets `it = end` then the caller does `++it` on an
    // already-end iterator, after which the loop dereferences it.
    {
        args::ArgumentParser p("parser");
        args::CompletionFlag c(p, {"completion"});
        args::MapFlag<std::string, int> m(
            p, "mappos", "mappos", {'m', "map"},
            {{"alpha", 1}, {"beta", 2}});

        p.ParseArgs(std::vector<std::string>{
            "--completion", "bash", "2", "test", "-m", ""});

        std::fprintf(stderr, "site1 ok: err=%d reply=[%s]\n",
                     static_cast<int>(p.GetError()),
                     args::get(c).c_str());
    }

    // -------- Site 2: Dangling iterator returned across stack frames --------
    // ParseCLI compares the return value of ParseArgs against std::end(args).
    // On the buggy build the returned iterator points into a destroyed local
    // `curArgs` vector inside the completion handler; the == compare is then
    // a dangling-iterator compare (UB).
    {
        args::ArgumentParser p("parser");
        args::CompletionFlag c(p, {"completion"});
        args::MapFlag<std::string, int> m(
            p, "mappos", "mappos", {'m', "map"},
            {{"alpha", 1}, {"beta", 2}});

        const char *argv[] = {
            "prog", "--completion", "bash", "2", "test", "-m", ""};
        const int argc = static_cast<int>(sizeof(argv) / sizeof(argv[0]));
        p.ParseCLI(argc, argv);

        std::fprintf(stderr, "site2 ok: err=%d reply=[%s]\n",
                     static_cast<int>(p.GetError()),
                     args::get(c).c_str());
    }

    std::fprintf(stderr, "done\n");
    return 0;
}
