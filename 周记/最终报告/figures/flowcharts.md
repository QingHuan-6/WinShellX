# WinShellX 流程图 Mermaid 代码

导出说明：复制各段代码到 https://mermaid.live ，导出 PNG 后按文件名保存到本目录。

---

## flow_main_loop.png — 主程序总体流程

```mermaid
flowchart TD
    A([程序启动]) --> B[加载别名与历史]
    B --> C[注册内置命令]
    C --> D[显示提示符]
    D --> E[读取用户输入]
    E --> F{输入为空?}
    F -->|是| D
    F -->|否| G[写入历史]
    G --> H[别名展开]
    H --> I[解析管道/重定向/后台]
    I --> J{内部命令?}
    J -->|是| K[调用命令处理函数]
    J -->|否| L[ExternalCommandRunner]
    K --> M[输出结果]
    L --> M
    M --> N{exit?}
    N -->|否| D
    N -->|是| O[保存别名与历史]
    O --> P([程序结束])
```

---

## flow_shell_main.png — 主控与命令调度

```mermaid
flowchart TD
    A[executeInput] --> B[parseShellInput]
    B --> C{含管道 | ?}
    C -->|是| D[执行左命令并捕获输出]
    D --> E[执行右命令并传入管道文本]
    C -->|否| F{后台 & ?}
    F -->|是| G[executeSingle 不等待]
    F -->|否| H[executeSingle 前台执行]
    E --> I[返回]
    G --> I
    H --> I
```

---

## flow_command_dispatch.png — 命令解析与注册

```mermaid
flowchart TD
    A[用户输入字符串] --> B[splitArguments 拆分参数]
    B --> C[命令名转小写]
    C --> D[CommandRegistry.find]
    D --> E{找到?}
    E -->|是| F[调用 handler]
    E -->|否| G[ExternalCommandRunner.run]
    F --> H[输出到控制台或重定向]
    G --> H
```

---

## flow_builtin_cmds.png — 内置命令

```mermaid
flowchart TD
    A[内置命令名] --> B{命令类型}
    B -->|cd| C[SetCurrentDirectory]
    B -->|dir| D[FindFirstFile 遍历]
    B -->|history| E[输出 history 向量]
    B -->|tasklist| F[CreateToolhelp32Snapshot]
    B -->|taskkill| G[OpenProcess + TerminateProcess]
    B -->|cls| H[FillConsoleOutputCharacter]
    B -->|exit| I[设置 running=false]
    C --> J[输出结果或错误]
    D --> J
    E --> J
    F --> J
    G --> J
    H --> J
    I --> J
```

---

## flow_external_cmd.png — 外部命令执行

```mermaid
flowchart TD
    A[命令行] --> B[解析可执行文件名与参数]
    B --> C[读取 PATHEXT 补扩展名]
    C --> D[搜索当前目录/PATH/系统目录]
    D --> E{找到文件?}
    E -->|否| F[输出 not recognized]
    E -->|是| G{批处理?}
    G -->|是| H[cmd.exe /c 包装]
    G -->|否| I[CreateProcess 直接启动]
    H --> J{等待退出?}
    I --> J
    J -->|前台| K[WaitForSingleObject]
    J -->|后台 &| L[显示 PID 立即返回]
```

---

## flow_pipe_redirect.png — 管道与重定向

```mermaid
flowchart TD
    A[parseShellInput] --> B{含 > ?}
    B -->|是| C[提取输出文件名]
    B -->|否| D{含 | ?}
    C --> D
    D -->|是| E[左命令输出到字符串/管道]
    E --> F[右命令从管道读取]
    D -->|否| G[直接执行单命令]
    F --> H{还有 > ?}
    H -->|是| I[写入文件]
    H -->|否| J[打印到屏幕]
    G --> J
```

---

## flow_history_alias.png — 历史与别名

```mermaid
flowchart TD
    A[程序启动] --> B[HistoryStore.load]
    A --> C[AliasStore.load]
    D[用户输入] --> E{首 token 是别名?}
    E -->|是| F[替换为别名定义]
    E -->|否| G[保持原输入]
    F --> H[执行命令]
    G --> H
    H --> I[追加到 history]
    J[程序退出] --> K[HistoryStore.save 去重]
    J --> L[AliasStore.save]
```

---

## flow_completion.png — 行编辑与补全

```mermaid
flowchart TD
    A[LineEditor.readLine] --> B[ReadConsoleInput 读键]
    B --> C{按键类型}
    C -->|Up/Down| D[翻阅 history]
    C -->|F7| E[列出匹配历史供选择]
    C -->|Tab| F[CompletionProvider.complete]
    C -->|Enter| G[返回输入行]
    F --> H{匹配历史?}
    H -->|是| I[补全历史命令]
    H -->|否| J{匹配命令名?}
    J -->|是| K[补全内置命令]
    J -->|否| L[补全路径 FindFirstFile]
```
