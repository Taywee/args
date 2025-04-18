from conans import ConanFile

class ArgsConan(ConanFile):
    name = "args"
    version = "6.4.7"
    url = "https://github.com/Taywee/args"
    description = "A simple header-only C++ argument parser library."
    license = "MIT"
    exports = ["LICENSE"]
    exports_sources = "args.hxx"

    def package(self):
        self.copy(pattern="LICENSE", dst="license")
        self.copy(pattern="args.hxx", dst="include")
