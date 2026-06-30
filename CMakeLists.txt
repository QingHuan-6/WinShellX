cmake_minimum_required(VERSION 3.16)

project(WinShellX LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(WINSHELLX_COMMON_SOURCES
    src/shell/CommandParser.cpp
    src/shell/CommandResolver.cpp
    src/shell/ShellInputParser.cpp
    src/shell/CommandRegistry.cpp
    src/utils/StringUtils.cpp
    src/utils/EnvUtils.cpp
)

add_executable(WinShellX
    src/main.cpp
    src/shell/Shell.cpp
    src/shell/AliasStore.cpp
    src/shell/CompletionProvider.cpp
    src/shell/ExternalCommandRunner.cpp
    src/shell/HistoryStore.cpp
    src/shell/JobManager.cpp
    src/shell/LineEditor.cpp
    src/shell/ConsoleControl.cpp
    ${WINSHELLX_COMMON_SOURCES}
    src/commands/BuiltInCommands.cpp
    src/commands/AliasCommand.cpp
    src/commands/BatCommand.cpp
    src/commands/CdCommand.cpp
    src/commands/ClsCommand.cpp
    src/commands/CopyCommand.cpp
    src/commands/DelCommand.cpp
    src/commands/DirCommand.cpp
    src/commands/EchoCommand.cpp
    src/commands/ExitCommand.cpp
    src/commands/FgCommand.cpp
    src/commands/HelpCommand.cpp
    src/commands/HistoryCommand.cpp
    src/commands/JobsCommand.cpp
    src/commands/KillJobCommand.cpp
    src/commands/MkdirCommand.cpp
    src/commands/MoveCommand.cpp
    src/commands/RenCommand.cpp
    src/commands/RmdirCommand.cpp
    src/commands/SetCommand.cpp
    src/commands/TaskKillCommand.cpp
    src/commands/TaskListCommand.cpp
    src/commands/TopCommand.cpp
    src/commands/TypeCommand.cpp
    src/commands/UnaliasCommand.cpp
    src/commands/WhereCommand.cpp
    src/utils/PathUtils.cpp
    src/utils/WinApiError.cpp
    src/utils/ConsoleStyle.cpp
    src/utils/GitUtils.cpp
)

add_executable(WinShellX_tests
    tests/test_main.cpp
    tests/parser_tests.cpp
    tests/command_tests.cpp
    ${WINSHELLX_COMMON_SOURCES}
    src/utils/ConsoleStyle.cpp
)

target_include_directories(WinShellX PRIVATE include)
target_include_directories(WinShellX_tests PRIVATE include tests)

target_link_libraries(WinShellX PRIVATE Psapi)

if(MSVC)
    target_compile_options(WinShellX PRIVATE /utf-8)
    target_compile_options(WinShellX_tests PRIVATE /utf-8)
endif()

enable_testing()
add_test(NAME WinShellX_tests COMMAND WinShellX_tests)
