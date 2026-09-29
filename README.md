# MAMEoXtras 2026 — Raiden II + A/B 连发

面向初代 Xbox 的 MAMEoXtras 2026 源码与构建工程。本分支包含 Raiden II 主集驱动修复、A/B 连发设置、Xbox 优化、补丁和构建报告。

## 目录

- `MAMEoXtras 2026 Src/`：完整工程源码
- `raiden2-ogxbox.patch`、`autofire.patch`：对应源码改动
- `build_xbe.py`、`build_xbe.cmd`：可重复运行的 XBE 构建脚本
- `编译报告_Raiden2.md`：构建环境、命令、输出和校验摘要
- `tests/`、`tools/`：源码检查与辅助工具
- `upstream/`：移植参考版本与来源记录

## 构建

在已配置初代 Xbox XDK 的 Windows 环境，从本目录运行：

```powershell
.\build_xbe.cmd --jobs 8
```

构建脚本会将结果放在 `dist/`。该目录属于构建产物，不纳入 Git。

## 版本说明

- Raiden II 的驱动改动以主集 `raiden2` 为目标，其他关联游戏保留原有驱动。
- A/B 连发可在启动器选项中分别开关，默认关闭。
- `vm-raiden2-recommended.txt` 是逐游戏虚拟内存参数建议；部署时应合并进完整 `vm.txt`，不要用单行文件覆盖整份配置。
- 编译输出和 ROM 不存放在本仓库中。
