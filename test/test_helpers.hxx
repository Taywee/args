/* Copyright (c) Taylor Richberger <taylor@axfive.net>
 * This code is released under the license described in the LICENSE file
 *
 * Minimal assertion helpers for per-test-executable ctest setup.
 * On failure, prints a short message and exits 1.
 */

#ifndef ARGS_TEST_HELPERS_HXX
#define ARGS_TEST_HELPERS_HXX

#include <cstdlib>
#include <exception>
#include <iostream>
#include <string>

namespace test
{

[[noreturn]] inline void fail(const std::string &msg)
{
    std::cerr << "test failed: " << msg << '\n';
    std::exit(1);
}

inline void require(bool cond)
{
    if (!cond)
    {
        fail("require()");
    }
}

inline void require_false(bool cond)
{
    if (cond)
    {
        fail("require_false()");
    }
}

template <typename F>
void require_nothrow(F &&f)
{
    try
    {
        f();
    }
    catch (const std::exception &e)
    {
        fail(std::string("require_nothrow threw: ") + e.what());
    }
    catch (...)
    {
        fail("require_nothrow threw a non-std::exception");
    }
}

template <typename Exception, typename F>
void require_throws_as(F &&f)
{
    try
    {
        f();
    }
    catch (const Exception &)
    {
        return;
    }
    catch (const std::exception &e)
    {
        fail(std::string("require_throws_as caught wrong type: ") + e.what());
    }
    catch (...)
    {
        fail("require_throws_as caught non-std::exception of wrong type");
    }
    fail("require_throws_as: nothing thrown");
}

template <typename F>
void require_throws_with(F &&f, const std::string &expected)
{
    try
    {
        f();
    }
    catch (const std::exception &e)
    {
        const std::string actual(e.what());
        if (actual != expected)
        {
            fail("require_throws_with mismatch:\nexpected:\n" + expected
                 + "\ngot:\n" + actual);
        }
        return;
    }
    catch (...)
    {
        fail("require_throws_with caught non-std::exception");
    }
    fail("require_throws_with: nothing thrown");
}

} // namespace test

#endif
