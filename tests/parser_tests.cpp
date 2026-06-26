#include "shell/CommandParser.h"
#include "shell/ShellInputParser.h"
#include "utils/EnvUtils.h"
#include "utils/StringUtils.h"

#include "test_framework.h"

#include <cstdlib>
#include <vector>

namespace {
void testParseEmptyInput() {
    ParsedCommand parsed = parseCommand("");
    EXPECT_TRUE(parsed.name.empty());
    EXPECT_TRUE(parsed.args.empty());
}

void testParseSimpleCommand() {
    ParsedCommand parsed = parseCommand("dir");
    EXPECT_EQ(parsed.name, "dir");
    EXPECT_TRUE(parsed.args.empty());
}

void testParseCommandWithArgs() {
    ParsedCommand parsed = parseCommand("cd C:\\Windows");
    EXPECT_EQ(parsed.name, "cd");
    EXPECT_EQ(parsed.args.size(), 1U);
    EXPECT_EQ(parsed.args[0], "C:\\Windows");
}

void testParseQuotedPath() {
    ParsedCommand parsed = parseCommand("cd \"C:\\Program Files\"");
    EXPECT_EQ(parsed.name, "cd");
    EXPECT_EQ(parsed.args.size(), 1U);
    EXPECT_EQ(parsed.args[0], "C:\\Program Files");
}

void testParseSinglePipe() {
    ShellInputPlan plan = parseShellInput("dir | find \"cpp\"");
    EXPECT_EQ(plan.pipeline.size(), 2U);
    EXPECT_EQ(plan.pipeline[0], "dir");
    EXPECT_EQ(plan.pipeline[1], "find \"cpp\"");
    EXPECT_TRUE(plan.outputFile.empty());
}

void testParseMultiPipe() {
    ShellInputPlan plan = parseShellInput("a | b | c");
    EXPECT_EQ(plan.pipeline.size(), 3U);
    EXPECT_EQ(plan.pipeline[0], "a");
    EXPECT_EQ(plan.pipeline[1], "b");
    EXPECT_EQ(plan.pipeline[2], "c");
}

void testParsePipeWithRedirect() {
    ShellInputPlan plan = parseShellInput("dir | find \"cpp\" > result.txt");
    EXPECT_EQ(plan.pipeline.size(), 2U);
    EXPECT_EQ(plan.outputFile, "result.txt");
}

void testParseQuotedPipeIgnored() {
    ShellInputPlan plan = parseShellInput("echo \"a|b\"");
    EXPECT_EQ(plan.pipeline.size(), 1U);
    EXPECT_EQ(plan.pipeline[0], "echo \"a|b\"");
}

void testParseBackgroundFlag() {
    ShellInputPlan plan = parseShellInput("notepad &");
    EXPECT_EQ(plan.pipeline.size(), 1U);
    EXPECT_EQ(plan.pipeline[0], "notepad");
    EXPECT_TRUE(plan.background);
}

void testExpandUserProfile() {
    char* value = std::getenv("USERPROFILE");
    if (value == nullptr) {
        return;
    }

    std::string expanded = expandEnvironmentVariables("cd %USERPROFILE%");
    EXPECT_EQ(expanded, std::string("cd ") + value);
}

void testExpandPathContainsPercent() {
    std::string expanded = expandEnvironmentVariables("%PATH%");
    EXPECT_TRUE(!expanded.empty());
    EXPECT_TRUE(expanded.find('%') == std::string::npos);
}
}

void registerParserTests() {
    RUN_TEST(testParseEmptyInput);
    RUN_TEST(testParseSimpleCommand);
    RUN_TEST(testParseCommandWithArgs);
    RUN_TEST(testParseQuotedPath);
    RUN_TEST(testParseSinglePipe);
    RUN_TEST(testParseMultiPipe);
    RUN_TEST(testParsePipeWithRedirect);
    RUN_TEST(testParseQuotedPipeIgnored);
    RUN_TEST(testParseBackgroundFlag);
    RUN_TEST(testExpandUserProfile);
    RUN_TEST(testExpandPathContainsPercent);
}
