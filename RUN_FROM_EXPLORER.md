# 双击运行说明

如果想从资源管理器双击运行 WinShellX，推荐双击：

```text
run_winshellx.bat
```

该脚本会自动切换到项目目录，然后启动：

```text
WinShellX.exe
```

程序退出后窗口会停留，方便查看输出。

如果 Windows 智能应用控制阻止 `WinShellX.exe`，原因通常是本地编译的 exe 没有数字签名。可以改用 Developer PowerShell 运行：

```powershell
cd D:\Acode\CSPractise
.\WinShellX.exe
```

