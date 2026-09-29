MAMEoXtras 2026 — Raiden II 移植及初代 Xbox 优化版，保留 A/B 连发
版本：2026-09-24

安装：
1. 在初代 Xbox 上复制一份完整的 MAMEoXtras 2026 运行目录，作为本版测试目录。
2. 将本包的 default.xbe、MAMEoX.xbe 放入该目录，替换复制出来的两个文件。
3. 在完整 vm.txt 中增加或替换一行：raiden2 4 16 65535
   其他游戏的条目保留。不要把仅有一行的建议文件当成完整 vm.txt 覆盖。
4. 运行 default.xbe。本版会自动重建驱动列表缓存。
5. 在 VMM 设置中关闭 ForceVMM，让每游戏 vm.txt 参数生效。
6. 使用 raiden2.zip（Raiden II 主集），不是此前测试的 raiden.zip（雷电一代）。
7. 从头启动游戏，依次检查投币、开始、移动、射击、炸弹、音乐/音效、图层和精灵。

移植依据：
https://github.com/liujiny/mame2003-plus-libretro-ps3/commit/3c1441bc8567bebd810046a03593c3b421bf08d7
https://github.com/liujiny/mame2003-plus-libretro-ps3/commit/938c448045d8eeec992d5036cfff687412850da1

范围：此次修复 raiden2 主集。原工程中的 raiden2a/b/c/e、Raiden DX、Zero Team
保留原有驱动和状态，未冒充已经修复。GAME_IMPERFECT_GRAPHICS/SOUND 标记保留。

优化：Raiden II 使用无损 4 位图像存储并修复透明判断；精灵 ROM 直接交错加载，
省去 8 MiB 解密临时副本；解密查表和精灵优先级列表保持输出一致。
使用 128 字节临时块原地解码，并通过 GFX_RAW 复用 ROM 内存，避免再次分配整套图像。
保留全部图层、精灵、调色板、游戏时钟和声音配置。

vm 参数是手动试验起点，尚未在真机验证容量或速度；备选值见 vm-raiden2-recommended.txt。
本次按用户要求只编译及做主机回归验证，不运行 xemu 或声称真机已通过。
实际结果、ROM 校验、编译命令和输出哈希见项目目录中的编译报告_Raiden2.md。

A/B 连发：使用启动器的 Auto Fire Options (A/B)，八个开关各自独立，默认关闭。
MAME 默认第二游戏动作常映射到 Xbox X；若要用 B 连发炸弹，请在 Input (this game)
中将 P1 Button 2 设为 J1 B。连发设置及其他游戏设置沿用相同 Title ID 的存档位置。
