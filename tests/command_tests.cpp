#include "shell/CommandRegistry.h"
#include "shell/ICommand.h"
#include "shell/ShellContext.h"

#include "test_framework.h"

#include <memory>
#include <string>
#include <vector>

namespace {
class MockCommand : public ICommand {
public:
    MockCommand(std::string commandName, std::string commandUsage, std::string commandDescription)
        : commandName_(std::move(commandName)),
          commandUsage_(std::move(commandUsage)),
          commandDescription_(std::move(commandDescription)) {}

    std::string name() const override { return commandName_; }
    std::string usage() const override { return commandUsage_; }
    std::string description() const override { return commandDescription_; }
    void execute(ShellContext&, const std::vector<std::string>&) const override {}

private:
    std::string commandName_;
    std::string commandUsage_;
    std::string commandDescription_;
};

void testRegistryAddAndFind() {
    CommandRegistry registry;
    registry.add(std::make_unique<MockCommand>("mock", "mock", "Mock command"));

    const ICommand* found = registry.find("mock");
    EXPECT_TRUE(found != nullptr);
    EXPECT_EQ(found->name(), "mock");
}

void testRegistryCaseInsensitiveFind() {
    CommandRegistry registry;
    registry.add(std::make_unique<MockCommand>("tasklist", "tasklist", "List processes"));

    EXPECT_TRUE(registry.find("TASKLIST") != nullptr);
    EXPECT_TRUE(registry.find("TaskList") != nullptr);
}

void testRegistryUnknownCommand() {
    CommandRegistry registry;
    registry.add(std::make_unique<MockCommand>("cd", "cd <path>", "Change directory"));

    EXPECT_TRUE(registry.find("missing") == nullptr);
}

void testRegistryNames() {
    CommandRegistry registry;
    registry.add(std::make_unique<MockCommand>("alpha", "alpha", "Alpha"));
    registry.add(std::make_unique<MockCommand>("beta", "beta", "Beta"));

    std::vector<std::string> names = registry.names();
    EXPECT_EQ(names.size(), 2U);
}
}

void registerCommandTests() {
    RUN_TEST(testRegistryAddAndFind);
    RUN_TEST(testRegistryCaseInsensitiveFind);
    RUN_TEST(testRegistryUnknownCommand);
    RUN_TEST(testRegistryNames);
}
