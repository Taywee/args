#include <iostream>
#include <vector>
#include <string>
#include <limits>

// Minimal version of the safe functions
namespace args
{
    template<typename T>
    bool SafeAdd(T a, T b, T& out)
    {
        if (std::is_unsigned<T>::value || true)
        {
            if (b > std::numeric_limits<T>::max() - a)
            {
                return false;
            }
        }
        out = a + b;
        return true;
    }

    template<typename T>
    bool SafeMultiply(T a, T b, T& out)
    {
        if (a == 0 || b == 0)
        {
            out = 0;
            return true;
        }

        if (std::is_unsigned<T>::value || true)
        {
            if (b > std::numeric_limits<T>::max() / a)
            {
                return false;
            }
        }
        out = a * b;
        return true;
    }

    namespace detail
    {
        template <typename T>
        std::string Join(const T& array, const std::string &delimiter)
        {
            // Calculate total size safely to avoid integer overflow and
            // excessive reallocations. If the total size would overflow,
            // return an empty string to fail safely.
            using size_type = std::string::size_type;
            size_type total = 0;
            const size_type delim_size = static_cast<size_type>(delimiter.size());
            size_type count = 0;

            for (const auto &element : array)
            {
                const size_type elem_size = static_cast<size_type>(element.size());
                if (!SafeAdd<size_type>(total, elem_size, total))
                {
                    std::cout << "FAIL: SafeAdd overflow in element sum" << std::endl;
                    return std::string();
                }
                ++count;
            }

            std::cout << "After loop: count=" << count << " total=" << total << std::endl;

            if (count > 0 && count > 1)
            {
                // total delimiters = count - 1 (safe since count > 1)
                const size_type delim_count = count - 1;

                size_type delim_total = 0;
                if (!SafeMultiply<size_type>(delim_count, delim_size, delim_total))
                {
                    std::cout << "FAIL: SafeMultiply overflow in delimiter" << std::endl;
                    return std::string();
                }

                std::cout << "Delimiters: count=" << delim_count << " delim_size=" << delim_size << " delim_total=" << delim_total << std::endl;

                if (!SafeAdd<size_type>(total, delim_total, total))
                {
                    std::cout << "FAIL: SafeAdd overflow in total+delimiter" << std::endl;
                    return std::string();
                }
            }

            std::cout << "Final total before reserve: " << total << std::endl;
            
            std::string res;
            try
            {
                res.reserve(total);
            }
            catch (...) {
                std::cout << "FAIL: reserve threw" << std::endl;
                return std::string();
            }

            bool first = true;
            for (const auto &element : array)
            {
                if (!first)
                {
                    res += delimiter;
                }
                res += element;
                first = false;
            }

            std::cout << "Final result: '" << res << "' (length=" << res.length() << ")" << std::endl;
            return res;
        }
    }
}

int main()
{
    std::vector<std::string> v1 = {"a", "b", "c"};
    std::cout << "Test 1: {a, b, c} with ', '" << std::endl;
    auto r1 = args::detail::Join(v1, ", ");
    std::cout << "Result: '" << r1 << "' (expected 'a, b, c')" << std::endl << std::endl;

    std::vector<std::string> v2 = {"alpha", "beta"};
    std::cout << "Test 2: {alpha, beta} with '\\n'" << std::endl;
    auto r2 = args::detail::Join(v2, "\n");
    std::cout << "Result: '" << r2 << "' (expected 'alpha\\nbeta')" << std::endl << std::endl;

    std::vector<std::string> v3 = {};
    std::cout << "Test 3: {} with ', '" << std::endl;
    auto r3 = args::detail::Join(v3, ", ");
    std::cout << "Result: '" << r3 << "' (expected '')" << std::endl << std::endl;

    std::vector<std::string> v4 = {"single"};
    std::cout << "Test 4: {single} with ', '" << std::endl;
    auto r4 = args::detail::Join(v4, ", ");
    std::cout << "Result: '" << r4 << "' (expected 'single')" << std::endl;

    return 0;
}
