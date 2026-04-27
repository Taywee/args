#include "args.hxx"
#include <iostream>

int main() {
    args::ArgumentParser parser("Test program");
    args::Flag help(parser, "help", "Display help", {'h', "help"});

    // Set very small width to trigger underflow
    parser.helpParams.width = 5;

    std::cout << parser;
    return 0;
}