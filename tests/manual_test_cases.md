# WinShellX Manual Test Cases

## Basic Commands

| Case | Input | Expected Result |
| --- | --- | --- |
| Show help | `help` | Lists all built-in commands |
| Empty input | Press Enter | Shows next prompt without error |
| Unknown command | `abc` | Prints unknown command error |
| Exit shell | `exit` | Program exits normally |

## Directory Commands

| Case | Input | Expected Result |
| --- | --- | --- |
| Show current directory | `cd` | Prints current working directory |
| Change to current directory | `cd .` | No error |
| Change to parent directory | `cd ..` | Prompt path changes |
| Invalid path | `cd X:\not_exist` | Prints Windows API error |
| List current directory | `dir` | Shows files, directories, count, and disk space |
| List explicit directory | `dir .` | Shows current directory contents |

## History

| Case | Input | Expected Result |
| --- | --- | --- |
| Show history | `history` | Shows commands entered in current session |
| Preserve order | `help`, `dir`, `history` | History is numbered in input order |

## Process Commands

| Case | Input | Expected Result |
| --- | --- | --- |
| List processes | `tasklist` | Shows process name, PID, and thread count |
| Missing PID | `taskkill` | Prints usage |
| Invalid PID | `taskkill abc` | Prints invalid PID |
| Nonexistent PID | `taskkill 999999999` | Prints OpenProcess error |

