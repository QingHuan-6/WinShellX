#include "test_framework.h"

int main() {
    registerParserTests();
    registerCommandTests();
    registerResolverTests();
    return runAllTests();
}
