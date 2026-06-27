#pragma once

#include <iostream>
#include <sstream>
#include <string>

struct TestFailure {
    const char* file;
    int line;
    std::string message;
};

inline int& testFailureCount() {
    static int count = 0;
    return count;
}

inline void reportFailure(const char* file, int line, const std::string& message) {
    ++testFailureCount();
    std::cerr << "[FAIL] " << file << ":" << line << " " << message << "\n";
}

#define EXPECT_TRUE(expr)                                                         \
    do {                                                                          \
        if (!(expr)) {                                                            \
            reportFailure(__FILE__, __LINE__, std::string("Expected true: ") + #expr); \
        }                                                                         \
    } while (0)

#define EXPECT_EQ(actual, expected)                                               \
    do {                                                                          \
        const auto _actual = (actual);                                            \
        const auto _expected = (expected);                                        \
        if (!(_actual == _expected)) {                                            \
            std::ostringstream _oss;                                              \
            _oss << "Expected: [" << _expected << "] Actual: [" << _actual << "]"; \
            reportFailure(__FILE__, __LINE__, _oss.str());                        \
        }                                                                         \
    } while (0)

#define RUN_TEST(testFn)                                                          \
    do {                                                                          \
        const int before = testFailureCount();                                    \
        testFn();                                                                 \
        if (testFailureCount() == before) {                                       \
            std::cout << "[PASS] " << #testFn << "\n";                            \
        }                                                                         \
    } while (0)

inline int runAllTests() {
    if (testFailureCount() == 0) {
        std::cout << "All tests passed.\n";
        return 0;
    }

    std::cerr << testFailureCount() << " test(s) failed.\n";
    return 1;
}

void registerParserTests();
void registerCommandTests();
void registerResolverTests();
