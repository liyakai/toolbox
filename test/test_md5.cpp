#include "unit_test_frame/unittest.h"
#include "tools/md5.h"
#include <thread>
#include <iostream>
#include <cassert>

FIXTURE_BEGIN(MD5Test)

CASE(TestMD5)
{
// Test cases from RFC 1321
    struct TestCase {
        std::string_view input;
        uint64_t expected_hash64;
        uint32_t expected_hash32;
    };

    const TestCase test_cases[] = {
        // Empty string
        {"", 0xd41d8cd98f000000ULL, 0xd41d8cd9},
        // Basic test cases
        {"a", 0x0cc175b9c0f1b6a8ULL, 0x0cc175b9},
        {"abc", 0x900150983cd24fb0ULL, 0x90015098},
        {"message digest", 0xf96b697d7cb7938dULL, 0xf96b697d},
        // Longer test cases
        {"abcdefghijklmnopqrstuvwxyz", 0xc3fcd3d76192e400ULL, 0xc3fcd3d7},
        {"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789", 
         0xd174ab98d277d9f5ULL, 0xd174ab98},
    };

    for (const auto& test : test_cases) {
        uint64_t hash64 = MD5Hash64Constexpr(test.input);
        uint32_t hash32 = MD5Hash32Constexpr(test.input);

        if (hash64 != test.expected_hash64 || hash32 != test.expected_hash32) {
            std::cout << "Test failed for input: " << test.input << "\n";
            std::cout << "Hash64 expected: 0x" << std::hex << test.expected_hash64 
                     << ", got: 0x" << hash64 << "\n";
            std::cout << "Hash32 expected: 0x" << test.expected_hash32 
                     << ", got: 0x" << hash32 << "\n";
            assert(false);
        }
    }

    std::cout << "All MD5 tests passed!\n";
}

FIXTURE_END(TestMD5)