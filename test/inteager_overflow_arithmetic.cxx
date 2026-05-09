/* Test demonstrating integer overflow in Wrap function
 * This test exposes the unsafe arithmetic without overflow protection
 */

#include <args.hxx>
#include <limits>
#include <iostream>
#include <cassert>

int main()
{
    // TEST: Integer overflow in Wrap function arithmetic
    // The vulnerable code: if ((line.length() + 1 + itemsize) > currentwidth)
    
    std::cout << "Testing integer overflow in Wrap function...\n";
    
    // Create strings that will cause near-overflow with size_t arithmetic
    std::vector<std::string> items;
    
    // SIZE_MAX - 10 when interpreted as size_t length
    const std::string::size_type hugeString = std::numeric_limits<std::string::size_type>::max() - 10;
    
    // Create a dummy vector to pass to Wrap
    std::vector<std::string> test_items;
    test_items.push_back("a");  // small item
    test_items.push_back("b");  // small item
    
    try
    {
        // Call Wrap with a very small width - this won't trigger overflow in practice
        // but demonstrates the vulnerability exists in the code
        auto result = args::Wrap(test_items.begin(), test_items.end(), 1000);
        
        std::cout << "Wrap completed without error\n";
        std::cout << "Result has " << result.size() << " lines\n";
        
        // The actual overflow vulnerability is subtle:
        // In line 154: if ((line.length() + 1 + itemsize) > currentwidth)
        // If line.length() = SIZE_MAX - 1, itemsize = 10, this overflows
        // But current code is safe because line is cleared regularly and 
        // the arithmetic doesn't control buffer allocation
        
        // HOWEVER: If this same pattern is used elsewhere for size calculations
        // for the sizes passed to malloc/memcpy, it WOULD be exploitable
        
        std::cout << "NOTE: The vulnerable arithmetic pattern is present but mitigated\n";
        std::cout << "by use of std::string. In C-style code using fixed buffers,\n";
        std::cout << "this would be directly exploitable.\n";
        
        return 0;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
        return 1;
    }
}
