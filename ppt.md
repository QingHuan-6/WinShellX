# WinShellX 答辩 PPT 内容规划

> 按你定的五章目录编写，共 **15 页**（含封面、目录、致谢）。  
> 建议比例 **16:9**，字体：标题黑体 32～36，正文微软雅黑 20～24。  
> 流程图源码见 `周记/最终报告/figures/flowcharts.md`，导出 PNG 后放入 `周记/最终报告/figures/` 或 `ppt_assets/`。

---

## 目录结构（与你 PPT 目录页一致）

| 章节 | 中文 | 英文 |
|------|------|------|
| 01 | 项目背景和目标 | Background & Target |
| 02 | 基础功能实现 | Basic Functions |
| 03 | 扩展亮点总览 | Extended Features / Highlights |
| 04 | 测试验证 | Test & Validation |
| 05 | 总结与展望 | Conclusion & Outlook |

---

## 第 1 页｜封面

**版式：** 居中标题 + 副标题 + 个人信息

**文字内容：**
- 主标题：**WinShellX —— Windows 命令行解释器设计与实现**
- 副标题：基于 Win32 API 的可扩展教学型 Shell
- 信息：姓名、学号、专业、指导教师、答辩日期

**配图：**
- 背景或右侧：**WinShellX 运行截图**（带彩色提示符、`help` 或 `dir` 输出）
- 路径建议：自行截图保存为 `ppt_assets/cover_shell.png`

**口播（约 15 秒）：**
> 各位老师好，我汇报的题目是 WinShellX，一个基于 Win32 API 的 Windows 命令行解释器。

---

## 第 2 页｜目录

**版式：** 与你现有设计一致（5 列竖线分隔 + 01～05 编号）

**文字内容：**
```
01  项目背景和目标          Background & Target
02  基础功能实现            Basic Functions
03  扩展亮点总览            Extended Features
04  测试验证                Test & Validation
05  总结与展望              Conclusion & Outlook
```

**配图：** 无（纯目录页）

**口播（约 10 秒）：**
> 汇报分五部分，重点在第 3 章扩展亮点和第 4 章测试与演示。

---

# 01 项目背景和目标

## 第 3 页｜实训背景与目的

**标题：** 01 项目背景和目标

**正文要点：**
- 实训任务：设计并实现简化版 **Windows 命令行解释器**
- 学习重点：理解 **用户程序 → Shell → Win32 API → 操作系统内核** 的层次关系
- 开发语言：**C++17**，构建工具 **CMake**，运行环境 **Windows 控制台**
- 设计定位：教学型 Shell，展示原理与工程能力，**不完全复刻 cmd.exe**

**配图：**
- **系统层次示意图**（自绘或用报告中的层次图）
  - 建议内容：用户 → WinShellX → Win32 API → 内核 → 硬件
  - 文件名建议：`ppt_assets/layer_diagram.png`
- 若无现成图：用 PPT  SmartArt「层次结构」即可

**口播（约 40 秒）：**
> 本实训要求实现命令解释器，通过 Win32 API 间接使用系统服务，加深对操作系统和系统调用的理解。

---

## 第 4 页｜项目定位与目标

**标题：** WinShellX 项目定位

**正文要点：**
- **Win** = Windows 平台
- **Shell** = 命令行解释器
- **X** = eXtensible，可扩展
- 运行方式：启动后显示 `当前目录>`，循环 **读入 → 解析 → 执行 → 输出**
- 交付物：源代码、可执行程序、设计文档、测试、答辩材料

**配图：**
- 左侧：命名含义三行图标
- 右侧：**主程序流程图**
  - 使用 `flow_main_loop.png`（来自 `flowcharts.md` 第 1 段 Mermaid 导出）

**口播（约 30 秒）：**
> 项目命名为 WinShellX，采用经典 REPL 主循环，后续所有功能都围绕这条链路扩展。

---

# 02 基础功能实现

## 第 5 页｜任务书要求的功能

**标题：** 02 基础功能实现 —— 内置命令

**正文要点（表格）：**

| 命令 | 功能 | 主要 API |
|------|------|----------|
| `cd` | 切换当前目录 | `SetCurrentDirectory` |
| `dir` | 列出文件/目录 | `FindFirstFile` / `FindNextFile` |
| `history` | 显示历史命令 | 内存向量 |
| `exit` | 退出解释器 | 修改运行状态 |
| `tasklist` | 枚举进程 | `CreateToolhelp32Snapshot` |
| `taskkill` | 终止进程 | `OpenProcess` + `TerminateProcess` |

**配图：**
- 右侧：**内置命令流程图** `flow_builtin_cmds.png`
- 可选小截图：`tasklist` 或 `dir` 控制台输出 → `ppt_assets/basic_dir.png`

**口播（约 45 秒）：**
> 以上命令覆盖实训任务书要求，均通过 Win32 API 实现，并对错误情况给出提示。

---

## 第 6 页｜总体架构（支撑基础与扩展）

**标题：** 模块化总体架构

**正文要点：**
- **Shell**：主控循环，负责调度
- **CommandParser / ShellInputParser**：解析命令、管道、重定向
- **CommandRegistry + ICommand**：内置命令注册与执行
- **ExternalCommandRunner**：外部程序启动
- **LineEditor / HistoryStore / AliasStore**：交互与持久化
- 设计原则：**高内聚、低耦合**，主循环不实现具体命令逻辑

**配图：**
- **架构分层图**（推荐自绘，比代码截图更清晰）：

```
┌─────────────────────────────────────┐
│  用户输入 / 控制台输出               │
├─────────────────────────────────────┤
│  Shell 主控 + LineEditor            │
├─────────────────────────────────────┤
│  解析层  CommandParser / ShellInputParser / EnvUtils │
├─────────────────────────────────────┤
│  执行层  CommandRegistry(ICommand) / ExternalCommandRunner │
├─────────────────────────────────────┤
│  Win32 API（目录·进程·CreateProcess） │
└─────────────────────────────────────┘
```

- 或使用 **命令分发流程图** `flow_command_dispatch.png`

**口播（约 40 秒）：**
> 程序按模块划分，后续扩展亮点都建立在这个架构之上，而不是堆在主函数里。

---

# 03 扩展亮点总览

> **本章 4～5 页，是答辩重点。** 目录页只写「扩展亮点总览」，正文逐条展开。

## 第 7 页｜六大扩展亮点全景

**标题：** 03 扩展亮点总览

**正文要点（六宫格或 2×3 表格）：**

| # | 亮点 | 说明 |
|---|------|------|
| ① | **ICommand 插件架构** | 一命令一类，`CommandRegistry` 注册，加命令不改主循环 |
| ② | **外部命令执行** | PATH / PATHEXT 搜索，`CreateProcess` 启动 |
| ③ | **管道 · 重定向 · 后台** | 多级 `\|`、`>`、`&` |
| ④ | **环境变量展开** | `%USERPROFILE%`、`%PATH%` 等 |
| ⑤ | **LineEditor 交互** | 历史、Tab 补全、F7、方向键、光标移动、彩色输出 |
| ⑥ | **别名 + 持久化** | `.winshellx_aliases` / `.winshellx_history` |
| （可选⑦） | **自动化测试** | `WinShellX_tests`，15 个用例 |

**配图：**
- 中心放 **WinShellX 截图**（`help` 彩色输出）
- 六亮点围绕排列；或用 6 个小图标

**口播（约 50 秒）：**
> 在任务书基础上，我实现了六类扩展，使 WinShellX 更接近真实 Shell 的使用体验和工程质量。

---

## 第 8 页｜亮点① 可扩展命令架构

**标题：** 亮点① ICommand 插件式命令架构

**正文要点：**
- 抽象接口 `ICommand`：`name()` / `usage()` / `description()` / `execute()`
- 每个命令独立类：`CdCommand`、`DirCommand`、`TaskListCommand`…
- `CommandRegistry` 维护 `命令名 → ICommand` 映射
- `BuiltInCommands.cpp` 统一注册
- **新增命令步骤**：新建类 → 实现接口 → 注册 → 加入 CMake

**配图：**
- **类关系简图**（PPT 自绘）：
  ```
  ICommand <<interface>>
      ↑
  CdCommand  DirCommand  HelpCommand ...
      ↓
  CommandRegistry ──find──> execute()
  ```
- 可选：代码截图 1 张（`ICommand.h` 接口定义，**不要贴大段代码**）

**口播（约 45 秒）：**
> 采用命令模式，符合开闭原则，体现 OOP 设计，便于答辩展示和后续扩展。

---

## 第 9 页｜亮点② 外部命令与环境变量

**标题：** 亮点② 外部命令 + 环境变量

**正文要点：**

**外部命令：**
- 内部命令未命中 → `ExternalCommandRunner`
- 读取 `PATHEXT`，搜索当前目录 / PATH / 系统目录
- `.exe/.com` 直接 `CreateProcess`；`.bat/.cmd` 用 `cmd.exe /c`

**环境变量：**
- `ExpandEnvironmentStrings` 展开 `%VAR%`
- 在别名展开之后、命令执行之前处理
- 示例：`cd %USERPROFILE%`

**配图：**
- **外部命令流程图** `flow_external_cmd.png`
- 截图：`cd %USERPROFILE%` 前后对比 → `ppt_assets/env_cd.png`
- 或：`notepad` 启动成功（可选）

**口播（约 45 秒）：**
> 不仅支持内置命令，还能像 cmd 一样调用系统程序，并支持环境变量路径。

---

## 第 10 页｜亮点③ 管道、重定向与后台

**标题：** 亮点③ 管道 · 重定向 · 后台任务

**正文要点：**

| 特性 | 示例 | 实现概要 |
|------|------|----------|
| 输出重定向 | `dir > out.txt` | 重定向 `stdout` 到文件 |
| 多级管道 | `dir \| find "cpp" \| find "h"` | 左命令捕获输出 → 右命令读 stdin |
| 管道+重定向 | `dir \| find "cpp" > r.txt` | 最后一级写文件 |
| 后台执行 | `notepad &` | 不等待，显示 PID |

**注意（可放小字）：**
- 内置命令（如 `help`）**不读取**管道输入；过滤请用 `find` 等外部命令

**配图：**
- **管道与重定向流程图** `flow_pipe_redirect.png`
- 截图：`dir | find "cpp"` 运行效果 → `ppt_assets/pipe_demo.png`
- 截图：`dir > out.txt` 及生成的文件（可选）

**口播（约 50 秒）：**
> 支持命令组合，体现对标准输入输出重定向和进程间数据传递的理解。

---

## 第 11 页｜亮点④ 交互增强（LineEditor）

**标题：** 亮点④ 行编辑器与智能补全

**正文要点：**
- `ReadConsoleInput` 读键，自定义行编辑
- **历史**：↑↓ 翻阅；**F7** 列出匹配历史并数字选择
- **Tab 补全**：历史命令 / 内置命令名 / 当前目录路径
- **灰色 hint**：输入前缀时显示最近匹配
- **光标**：←→、Home、End；Backspace / Delete 在光标处编辑
- **彩色输出**：提示符、目录名、错误信息分色（`SetConsoleTextAttribute`）
- 持久化：退出保存历史与别名，下次启动加载

**配图：**
- **行编辑与补全流程图** `flow_completion.png`
- **历史与别名流程图** `flow_history_alias.png`（可缩小放角落）
- 截图（重要，建议自己截）：
  - `ppt_assets/tab_hint.png` — Tab 补全 + 灰色 hint
  - `ppt_assets/f7_history.png` — F7 历史选择界面
  - `ppt_assets/color_dir.png` — `dir` 彩色目录名

**口播（约 50 秒）：**
> 交互体验是本项目最大的差异化，从简单 getline 升级为接近日常 Shell 的行编辑器。

---

## 第 12 页｜亮点⑤ 别名系统（可选，或与第 11 页合并）

**标题：** 亮点⑤ 命令别名

**正文要点：**
- `alias ll=dir` 创建别名；`alias` 列出；`unalias ll` 删除
- 启动时 `AliasStore.load`，退出时 `save`
- 执行前展开别名（`alias` / `unalias` 命令本身不展开）

**配图：**
- 终端截图：`alias ll=dir` → `ll` 效果 → `ppt_assets/alias_demo.png`

**口播（约 25 秒）：**
> 别名提升易用性，并持久化到文件，重启后仍有效。

> 若页数要压缩：把本页合并进第 11 页，用半页截图即可。

---

# 04 测试验证

## 第 13 页｜测试与现场演示

**标题：** 04 测试验证

**正文要点：**

**自动化测试：**
- 可执行文件：`build/Debug/WinShellX_tests.exe`
- 一键脚本：`test.bat`
- `parser_tests.cpp`：空输入、带引号路径、单/多级管道、重定向、`%VAR%` 展开
- `command_tests.cpp`：注册、查找、大小写不敏感
- 结果：**15 个用例全部 PASS**

**手工测试：**
- 见 `tests/manual_test_cases.md`
- 覆盖 cd、taskkill、管道、别名、交互等

**现场演示脚本（答辩必做）：**
```text
help
cd %USERPROFILE%
dir | find "Desktop"
alias ll=dir
ll
tasklist | find "WinShellX"
exit
```

**配图：**
- 截图：`test.bat` 或 `WinShellX_tests.exe` 全部 PASS → `ppt_assets/test_pass.png`
- 可选：演示用的终端全屏截图拼贴

**口播（约 60 秒，含演示）：**
> 除手工测试外，实现了自动化测试保证解析和注册逻辑正确；下面简要演示主要功能。

---

# 05 总结与展望

## 第 14 页｜总结与展望

**标题：** 05 总结与展望

**正文要点：**

**完成工作：**
- 任务书规定的内置命令与 Win32 API 调用 ✓
- 六项扩展亮点：架构、外部命令、管道、环境变量、交互、测试 ✓
- 模块化 C++ 工程，CMake 构建，`build.bat` / `test.bat` 一键使用 ✓

**实训收获：**
- 理解命令解释器与系统调用层次
- 掌握 Win32 目录、进程、进程创建 API
- 实践 OOP 命令模式与 Shell 交互设计

**不足与展望：**
- 暂不支持多行续行（`\` / `^`）
- 内置命令暂不支持读管道 stdin
- 后续可扩展：宽字符 API、作业控制、更多脚本语法

**配图：**
- 左侧：要点列表
- 右侧：项目 Logo 或 `WinShellX` 运行截图（与封面呼应）

**口播（约 40 秒）：**
> 本项目在完成任务书基础上做了多项扩展；仍有改进空间，请老师们批评指正。

---

## 第 15 页｜致谢

**文字内容：**
- **谢谢各位老师！**
- **敬请批评指正**
- Q & A

**配图：** 简洁背景即可，或与封面风格统一

---

# 附录：配图清单与来源

## A. 流程图（从 Mermaid 导出 PNG）

| 文件名 | 用于页码 | 源码位置 |
|--------|----------|----------|
| `flow_main_loop.png` | 第 4 页 | `flowcharts.md` §主程序总体流程 |
| `flow_command_dispatch.png` | 第 6 页 | `flowcharts.md` §命令解析与注册 |
| `flow_builtin_cmds.png` | 第 5 页 | `flowcharts.md` §内置命令 |
| `flow_external_cmd.png` | 第 9 页 | `flowcharts.md` §外部命令 |
| `flow_pipe_redirect.png` | 第 10 页 | `flowcharts.md` §管道与重定向 |
| `flow_completion.png` | 第 11 页 | `flowcharts.md` §行编辑与补全 |
| `flow_history_alias.png` | 第 11 页 | `flowcharts.md` §历史与别名 |
| `flow_shell_main.png` | 备用 | `flowcharts.md` §主控与命令调度 |

导出步骤：复制 Mermaid 到 https://mermaid.live → Export PNG → 存到 `ppt_assets/` 或报告 `figures/`。

## B. 控制台截图（需自行截取）

建议在 **Release 版**、**全屏终端**、**字体放大** 后截图：

| 文件名 | 内容 | 用于页码 |
|--------|------|----------|
| `cover_shell.png` | 启动界面 + 提示符 | 第 1 页 |
| `basic_dir.png` | `dir` 输出 | 第 5 页 |
| `env_cd.png` | `cd %USERPROFILE%` | 第 9 页 |
| `pipe_demo.png` | `dir \| find "cpp"` | 第 10 页 |
| `tab_hint.png` | 输入 `di` 时 Tab/hint | 第 11 页 |
| `f7_history.png` | 按 F7 后的历史列表 | 第 11 页 |
| `color_dir.png` | 目录名彩色显示 | 第 11 页 |
| `alias_demo.png` | `alias ll=dir` + `ll` | 第 12 页 |
| `test_pass.png` | `test.bat` 全部 PASS | 第 13 页 |

截图命令参考：
```powershell
cd D:\Acode\CSPractise
.\build\Release\WinShellX.exe
# 或 .\run_winshellx.bat
```

## C. 建议新建的文件夹

```text
ppt_assets/
├── cover_shell.png
├── basic_dir.png
├── ...
└── （流程图 PNG 副本）
```

---

# 附录：页数压缩方案（若限制 12 页）

| 合并方式 | 结果 |
|----------|------|
| 第 3+4 页合并 | 背景与定位一页 |
| 第 12 页并入第 11 页 | 别名用半页截图 |
| 致谢与总结合并 | 第 14 页下方加「谢谢」 |

压缩后：**12 页**，仍保留扩展亮点 3 页（第 7～11 页核心内容）。

---

# 附录：答辩时间分配（约 7 分钟）

| 部分 | 页码 | 时间 |
|------|------|------|
| 封面 + 目录 | 1～2 | 0.5 min |
| 01 背景目标 | 3～4 | 1 min |
| 02 基础功能 + 架构 | 5～6 | 1 min |
| **03 扩展亮点** | **7～12** | **2.5 min** |
| 04 测试 + 演示 | 13 | 1.5 min |
| 05 总结 | 14～15 | 0.5 min |

---

*文档版本：与 WinShellX 当前代码一致（含 ICommand、多级管道、环境变量、LineEditor 光标、自动化测试）。*
