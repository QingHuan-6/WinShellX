# WinShellX Manual Test Cases

## Basic Commands

| Case | Input | Expected Result |
| --- | --- | --- |
| Show help | `help` | Lists all built-in commands |
| Clear screen | `cls` | Console content is cleared and prompt returns to top |
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
| Save history | Enter `help`, then `exit` | `.winshellx_history` is updated |
| Load history | Restart WinShellX, then run `history` | Previous session commands are shown |
| History hint | Type `he` after running `help` before | The rest of `help` is shown as a hint |
| Accept hint | Type `he`, then press `Tab` | The input becomes `help` |
| Complete command name | Type `ta`, then press `Tab` | Input completes to a matching command prefix or command |
| Complete path | Type `dir s`, then press `Tab` | Input completes from matching files or directories |
| Choose recent history | Type a prefix, then press `F7` | Matching recent commands are listed |
| Select history item | Press a number from the `F7` list | The selected command is filled into the input line |

## Process Commands

| Case | Input | Expected Result |
| --- | --- | --- |
| List processes | `tasklist` | Shows process name, PID, and thread count |
| Missing PID | `taskkill` | Prints usage |
| Invalid PID | `taskkill abc` | Prints invalid PID |
| Nonexistent PID | `taskkill 999999999` | Prints OpenProcess error |

## External Commands

| Case | Input | Expected Result |
| --- | --- | --- |
| Start Notepad | `notepad` | Notepad starts, WinShellX continues after it exits |
| Start Calculator | `calc` | Calculator starts, WinShellX continues after it exits |
| Start program with argument | `notepad test.txt` | Notepad opens or creates `test.txt` |
| Invalid external command | `not_a_real_program` | Prints CreateProcess error |
