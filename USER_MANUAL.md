# WinShellX 使用手册

## 1. 程序启动

WinShellX 是一个运行在 Windows 控制台中的命令行解释器。

推荐双击运行：

```text
run_winshellx.bat
```

也可以在 PowerShell 或 Developer PowerShell 中运行：

```powershell
cd D:\Acode\CSPractise
.\WinShellX.exe
```

启动后会看到类似提示：

```text
WinShellX started. Type help for commands.
D:\Acode\CSPractise>
```

其中 `D:\Acode\CSPractise>` 表示当前工作目录和命令提示符。

## 2. 查看帮助

输入：

```text
help
```

效果：

```text
WinShellX commands:
  alias [name=command] Create or list command aliases
  cd <path>            Change current directory
  cls                  Clear console screen
  dir [path]           List files and directories
  exit                 Exit WinShellX
  help                 Show command help
  history              Show command history
  taskkill <pid>       Terminate process by PID
  tasklist             List running processes
  unalias <name>       Remove command alias
```

## 3. 目录命令

### 3.1 查看当前目录

输入：

```text
cd
```

效果：

```text
D:\Acode\CSPractise
```

### 3.2 切换目录

输入：

```text
cd ..
```

效果：当前目录切换到上一级目录。

输入：

```text
cd "D:\Acode\CSPractise"
```

效果：切换到指定目录。路径中有空格时建议使用双引号。

### 3.3 显示目录内容

输入：

```text
dir
```

效果：显示当前目录下的文件、目录、文件数量、目录数量和磁盘空间。

输入：

```text
dir src
```

效果：显示 `src` 目录下的内容。

## 4. 清屏与退出

清空控制台：

```text
cls
```

退出 WinShellX：

```text
exit
```

## 5. 历史命令

### 5.1 查看历史

输入：

```text
history
```

效果：

```text
   1  help
   2  dir
   3  cd ..
```

历史命令会保存到 `.winshellx_history`，下次启动后仍可查看。

### 5.2 上下键翻历史

在命令提示符下：

```text
按 Up
```

效果：显示上一条历史命令。

```text
按 Down
```

效果：显示下一条历史命令，或恢复当前输入草稿。

### 5.3 F7 选择最近历史

输入命令前缀：

```text
di
```

然后按：

```text
F7
```

效果：显示最近匹配的历史命令列表，输入数字即可选择。

## 6. 智能补全

WinShellX 支持 `Tab` 补全。

### 6.1 补全历史命令

如果之前输入过：

```text
tasklist
```

再次输入：

```text
ta
```

按 `Tab`，可能补全为：

```text
tasklist
```

### 6.2 补全内置命令

输入：

```text
his
```

按 `Tab`，补全为：

```text
history
```

### 6.3 补全路径

如果当前目录下有 `src` 文件夹，输入：

```text
dir s
```

按 `Tab`，补全为：

```text
dir src\
```

## 7. 进程管理

### 7.1 查看进程

输入：

```text
tasklist
```

效果：显示系统当前进程信息，包括进程名、PID 和线程数。

示例：

```text
Image Name                                 PID     Threads
----------------------------------------------------------
explorer.exe                             39440         379
Code.exe                                491548          83
```

### 7.2 结束进程

输入：

```text
taskkill 1234
```

效果：尝试结束 PID 为 `1234` 的进程。

如果权限不足或进程不存在，会显示错误信息。

## 8. 外部命令执行

如果输入的不是内部命令，WinShellX 会按类似 `cmd.exe` 的方式查找外部程序：

1. 解析第一个 token 作为命令名。
2. 根据 `PATHEXT` 尝试 `.COM`、`.EXE`、`.BAT`、`.CMD` 等扩展名。
3. 在当前目录、系统目录、Windows 目录和 `PATH` 中搜索。
4. 对 `.exe/.com` 直接使用 `CreateProcess` 启动。
5. 对 `.bat/.cmd` 使用 `cmd.exe /c` 解释执行。

### 8.1 启动记事本

输入：

```text
notepad
```

效果：打开 Windows 记事本。

### 8.2 用 VS Code 打开当前目录

输入：

```text
code .
```

效果：如果 VS Code 已加入 PATH，会打开当前目录。

### 8.3 用 Cursor 打开当前目录

输入：

```text
cursor .
```

效果：如果 Cursor 已加入 PATH，会打开当前目录。

### 8.4 执行 cmd 命令

输入：

```text
cmd /c echo hello
```

效果：

```text
hello
```

## 9. 输出重定向

### 9.1 内部命令重定向

输入：

```text
dir > out.txt
```

效果：不在屏幕显示 `dir` 结果，而是把结果写入 `out.txt`。

查看文件：

```text
notepad out.txt
```

### 9.2 外部命令重定向

输入：

```text
cmd /c echo hello > out.txt
```

效果：`out.txt` 中保存：

```text
hello
```

## 10. 管道

WinShellX 支持单级管道。

### 10.1 内部命令传给外部命令

输入：

```text
dir | find "cpp"
```

效果：先执行 `dir`，再把输出传给 `find "cpp"`，只显示包含 `cpp` 的行。

### 10.2 管道后重定向

输入：

```text
dir | find "cpp" > result.txt
```

效果：过滤结果保存到 `result.txt`。

## 11. 命令别名

### 11.1 创建别名

输入：

```text
alias ll=dir
```

效果：

```text
Alias set: ll=dir
```

之后输入：

```text
ll
```

等价于：

```text
dir
```

### 11.2 查看别名

输入：

```text
alias
```

效果：

```text
ll=dir
```

### 11.3 查看指定别名

输入：

```text
alias ll
```

效果：

```text
ll=dir
```

### 11.4 删除别名

输入：

```text
unalias ll
```

效果：

```text
Alias removed: ll
```

别名会保存到 `.winshellx_aliases`，下次启动后仍然有效。

## 12. 后台任务

外部命令末尾加 `&` 可以后台启动。

输入：

```text
notepad &
```

效果：启动记事本后立即返回 WinShellX，并显示后台进程 PID：

```text
Started background process, PID: 12345
```

也可以输入：

```text
code . &
```

效果：后台打开 VS Code 当前目录。

注意：后台模式目前只支持外部命令，不支持内部命令、管道和重定向。

## 13. 彩色输出

WinShellX 使用 Windows Console API 提供彩色输出：

1. 提示符使用青绿色。
2. 帮助标题使用亮色。
3. `dir` 中的目录名使用醒目颜色。
4. 成功提示使用绿色。
5. 错误提示使用红色。

该功能用于提升交互体验，也体现了对 Windows 控制台 API 的调用。

## 14. 常见错误

### 14.1 命令不存在

输入：

```text
not_a_real_program
```

效果：

```text
'not_a_real_program' is not recognized as an internal or external command.
```

### 14.2 路径不存在

输入：

```text
cd X:\not_exist
```

效果：显示 Windows API 返回的错误信息。

### 14.3 taskkill 参数错误

输入：

```text
taskkill abc
```

效果：

```text
Invalid PID: abc
```

## 15. 推荐演示流程

答辩或验收时，可以按下面顺序演示：

```text
help
dir
cd ..
cd CSPractise
history
alias ll=dir
ll
dir > out.txt
notepad out.txt
dir | find "cpp"
tasklist
notepad &
cls
exit
```

这套流程可以覆盖内部命令、历史记录、别名、重定向、管道、进程管理、外部命令、后台任务和清屏功能。

