MAMEoXtras 2026 — 初代 Xbox A/B 连发版

使用方法
1. 在游戏列表按 START，选择 Options Menu。
2. 进入 General Options 后按右扳机，切换至 Auto Fire Options (A/B)。
3. 上下选择 P1～P4 对应手柄的 A 或 B。
4. 方向右或 A：Enabled（开）；方向左或 B：Disabled（关）。
   修改后立即通过原有 MAMEoX.ini 保存机制保存。
5. 按 START 打开菜单，选择 ROM List，返回游戏列表启动游戏。

行为说明
- 默认全部关闭。四个手柄的 A、B 共八个开关独立设置。
- Enabled 时按住对应实体按钮即可连发，松手停止；重新按下立即触发。
- 3 个游戏输入帧按下、3 帧释放：60 FPS 约 10 次/秒，50 FPS 约 8.3 次/秒。
  速度跟随游戏模拟帧率，视频跳帧不会额外加快连发。
- 设置跟随实体手柄端口，不跟随游戏内重映射后的玩家编号。
- A/B 只有映射为游戏数字按钮（Button 1～10）时才连发。
  方向、投币、开始、模拟轴、启动器菜单、MAME 设置菜单和按键重映射读原始输入。
- 例如默认 Xbox B 常映射 MAME Button 3；本功能始终识别实体 B。
- 这是启动器中的全局设置，适用于随后启动的游戏；不是逐游戏设置。
  游戏中可用 BACK + START 回到启动器后调整。

手动配置
在程序实际使用的 MAMEoX.ini 的 [Input] 节中设置 AutoFireP1A、AutoFireP1B，
以此类推至 AutoFireP4A、AutoFireP4B。0=关，1=开。缺少键时按关闭处理。
参考 AutoFire-defaults.ini.example；不要用示例文件覆盖完整 INI。
程序中的配置路径为 T:\SYSTEM\MAMEoX.ini（T: 是 Xbox 的标题存档映射）。
本版保持原 Title ID，两个版本可能共用原有存档和配置，新选项原版会忽略。

安装
将 default.xbe 与 MAMEoX.xbe 一起放入一份完整的 MAMEoXtras 2026 安装目录，
启动 default.xbe。建议在 Xbox 上也复制完整安装到独立目录再替换两个 XBE。
保留完整发行版的 vm.txt 和运行资源；本源码包未附 vm.txt，输出包不是完整发行包。

验证范围
已提供可重跑的主机输入逻辑测试和 XBE 结构校验；尚未进行 Xbox 真机运行测试。
本机源码、构建脚本、中间文件和输出均放在独立的 MAMEoXtras-2026-AB-Autofire 目录。
