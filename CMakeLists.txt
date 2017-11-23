# Copyright (c) 2016-2017 Taylor C. Richberger <taywee@gmx.com> and Pavel
# Belikov
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to
# deal in the Software without restriction, including without limitation the
# rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
# sell copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
# IN THE SOFTWARE.

cmake_minimum_required(VERSION 3.2)
project(args CXX)

add_library(args INTERFACE)
target_include_directories(args INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}")

add_executable(argstest test.cxx)
target_link_libraries(argstest args)
set_property(TARGET argstest PROPERTY CXX_STANDARD 11)

if (MSVC)
    target_compile_options(argstest PRIVATE /W4 /WX /bigobj)
else ()
    target_compile_options(argstest PRIVATE -Wall -Wextra -Werror -pedantic -Wshadow -Wunused-parameter)
endif ()

add_executable(argstest-multiple-inclusion test/multiple_inclusion_1.cxx test/multiple_inclusion_2.cxx)
target_link_libraries(argstest-multiple-inclusion args)
set_property(TARGET argstest-multiple-inclusion PROPERTY CXX_STANDARD 11)

enable_testing()
add_test(NAME "test" COMMAND argstest)
add_test(NAME "test-multiple-inclusion" COMMAND argstest-multiple-inclusion)
