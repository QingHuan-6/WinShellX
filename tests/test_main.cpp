#include "test_framework.h"

int main() {
    registerParserTests();
    registerCommandTests();
    return runAllTests();
}
