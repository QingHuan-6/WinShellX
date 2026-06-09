cmake_minimum_required(VERSION 3.16)

project(WinShellX LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(WinShellX
    src/main.cpp
    src/shell/Shell.cpp
    src/shell/CompletionProvider.cpp
    src/shell/HistoryStore.cpp
    src/shell/LineEditor.cpp
    src/shell/CommandParser.cpp
    src/shell/CommandRegistry.cpp
    src/commands/BuiltInCommands.cpp
    src/utils/StringUtils.cpp
    src/utils/PathUtils.cpp
    src/utils/WinApiError.cpp
)

target_include_directories(WinShellX PRIVATE include)
