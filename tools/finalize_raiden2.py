"""Verify the final build, record provenance/diffs and package the Chinese report."""
from pathlib import Path
import difflib
import hashlib
import json
import re
import shutil
import sys
from zipfile import ZIP_DEFLATED, ZipFile

ROOT = Path(__file__).resolve().parents[1]
BASE = ROOT.parent / "MAMEoXtras-2026-AB-Autofire"
sys.path.insert(0, str(ROOT))
import build_xbe as build
from verify_xbe import inspect

def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()

def text(path):
    return path.read_text(encoding="latin1")

def main():
    output = ROOT / "dist/MAMEoXtras-2026-Raiden2-AB"
    log = (ROOT / "build-raiden2-final.log").read_text(encoding="utf-16")
    assert "Package:" in log, "Wait for the final build and image packaging to complete."
    assert "errors 0" in log
    xbes = [inspect(output / filename) for filename in ("default.xbe", "MAMEoX.xbe")]
    assert all(x["encoding"] == "retail" and x["section_hashes_valid"] for x in xbes)
    header_stamp = max(p.stat().st_mtime_ns for pattern in ("*.h", "*.inc") for p in build.SRC.rglob(pattern))
    count = 0
    warnings = []
    for project in build.PROJECTS:
        for directory, source, obj, args in build.sources(project):
            expected = json.dumps([args, source.stat().st_mtime_ns, header_stamp])
            assert obj.is_file() and obj.with_suffix(".json").read_text() == expected, f"Stale object: {source}"
            unit_log = obj.with_suffix(".log").read_text(encoding="mbcs", errors="replace")
            assert not re.search(r"\b(?:fatal )?error [CL][0-9]+", unit_log)
            warnings += [{"source": str(source.relative_to(ROOT)), "message": line}
                         for line in unit_log.splitlines() if "warning " in line]
            count += 1
    assert count == 1955
    for name in ("raiden2", "seibu", "tilemap"):
        for file in (ROOT / "build/obj/MAME").glob(name + "-*.log"):
            assert "warning " not in file.read_text(encoding="mbcs", errors="replace"), file
    link_map = text(ROOT / "build/MAMEoX.map")
    for symbol in ("_driver_raiden2", "_video_start_r2play", "_video_update_r2play"):
        assert symbol in link_map
    tests = json.loads((ROOT / "build/raiden2-tests/results.json").read_text())
    assert set(tests) == {"graphics", "tilemap", "tilemap-negative", "sprites", "decrypt"}
    assert all(value["exit_code"] == (1 if value["expected_failure"] else 0) for value in tests.values())

    # Verify the prior AB binaries against their own pre-existing build manifest.
    unchanged = []
    for entry in json.loads((BASE / "build/xbe-verification.json").read_text()):
        path = BASE / "dist/MAMEoXtras-2026-AB-Autofire" / Path(entry["file"]).name
        assert sha(path) == entry["sha256"]
        unchanged.append({"file": str(path), "sha256": sha(path)})
    autofire_files = ["MAME/src/inptport.c", "MAMEoX/Includes/AutoFireState.h", "MAMEoX/Includes/xbox_AutoFire.h",
                      "MAMEoX/Sources/MAMEoXUtil.cpp", "MAMEoX/Sources/xbox_JoystickMouse.c",
                      "MAMEoXLauncher/Includes/OptionsScreen.h", "MAMEoXLauncher/Sources/OptionsScreen.cpp"]
    for name in autofire_files:
        assert sha(build.SRC / name) == sha(BASE / "MAMEoXtras 2026 Src" / name), name

    changes, patches = [], []
    for path in sorted(build.SRC.rglob("*")):
        if not path.is_file():
            continue
        relative = path.relative_to(ROOT)
        old = BASE / relative
        if old.is_file() and sha(path) == sha(old):
            continue
        if path.suffix.lower() not in (".c", ".h", ".inc", ".vcproj"):
            raise RuntimeError(f"Unexpected non-source change: {relative}")
        before = text(old) if old.exists() else ""
        after = text(path)
        patches.extend(difflib.unified_diff(before.splitlines(True), after.splitlines(True),
                       fromfile="a/" + relative.as_posix() if old.exists() else "/dev/null",
                       tofile="b/" + relative.as_posix()))
        changes.append({"file": relative.as_posix(), "before_sha256": sha(old) if old.exists() else None, "after_sha256": sha(path)})
    path = ROOT / "build_xbe.py"
    patches.extend(difflib.unified_diff((BASE / path.name).read_text(encoding="utf-8").splitlines(True),
                   path.read_text(encoding="utf-8").splitlines(True), fromfile="a/build_xbe.py", tofile="b/build_xbe.py"))
    for filename in ("README_RAIDEN2.txt", "vm-raiden2-recommended.txt"):
        patches.extend(difflib.unified_diff([], (ROOT / filename).read_text(encoding="utf-8").splitlines(True),
                       fromfile="/dev/null", tofile="b/" + filename))
    (ROOT / "raiden2-ogxbox.patch").write_text("".join(patches), encoding="utf-8", newline="\n")
    (ROOT / "source-changes-raiden2.json").write_text(json.dumps(changes, indent=2) + "\n")
    assert len(changes) == 13, f"Review unexpected changed file count: {len(changes)}"
    legacy = text(BASE / "MAMEoXtras 2026 Src/MAME/src/drivers/raiden2.c")
    driver = text(build.SRC / "MAME/src/drivers/raiden2.c")
    old_games = [line for line in legacy.splitlines() if line.startswith("GAMEX(") and not re.match(r"GAMEX\(\s*1993,\s*raiden2,", line)]
    assert all(line in driver for line in old_games) and len(old_games) == 10
    rom = re.search(r"ROM_START\( raiden2 \)(.*?)ROM_END", driver, re.S).group(1)
    roms = [{"file": name, "size": int(size, 16), "crc32": crc, "sha1": digest}
            for name, size, crc, digest in re.findall(r'ROM_LOAD\w*\("([^\"]+)",\s*0x[0-9a-f]+,\s*(0x[0-9a-f]+),\s*CRC\(([0-9a-f]+)\) SHA1\(([0-9a-f]+)\)', rom)]
    assert len(roms) == 12
    (ROOT / "raiden2-rom-requirements.json").write_text(json.dumps(roms, indent=2) + "\n")
    artifact_rows = "\n".join(f"| `{Path(x['file']).name}` | {x['bytes']:,} | `{x['sha256']}` |" for x in xbes)
    rom_rows = "\n".join(f"| `{x['file']}` | {x['size']:,} | `{x['crc32']}` |" for x in roms)
    changed_rows = "\n".join(f"- `{x['file']}`" for x in changes)
    report = f'''# MAMEoXtras 2026：Raiden II 移植、初代 Xbox 优化与编译报告

日期：2026-09-24。目标：初代 Xbox（x86 / 64 MiB），Release XBE。

## 交付结论

已移植指定两次提交中的 Raiden II 主集驱动及透明度/精灵锁存修复，并完成保持像素输出一致的内存与绘制优化。
**{count} 个编译单元全部通过，0 个错误；生成 default.xbe 和 MAMEoX.xbe。**
编译日志有 {len(warnings)} 条既有代码警告；本次 Raiden II、Seibu、tilemap 编译单元无警告。
XBE 结构、分段哈希、入口、内核导入均通过校验；已检查全部对象文件与当前源码、头文件、inc 文件和编译选项匹配。

**这是供用户手动试验的版本。本次未运行 xemu、未做真机或实际 ROM 游戏运行测试，因此不宣称实机可玩、满速或已通关。**
自动验证使用真实生产代码及合成输入，证明下述优化的解密结果、像素、透明掩码与精灵绘制顺序一致；仍需手测整机整合效果。

- 独立工程：`D:\\ogxbox mameexotra\\MAMEoXtras-2026-Raiden2-AB`
- 运行文件：`dist/MAMEoXtras-2026-Raiden2-AB/`
- 运行包：`dist/MAMEoXtras-2026-Raiden2-AB-XBE.zip`
- 相对原 A/B 连发工程的补丁：`raiden2-ogxbox.patch`
- 源码变化及哈希：`source-changes-raiden2.json`
- A/B 连发的 7 个实现文件逐字节保持一致；原 A/B 版两个 XBE 已按其原有清单确认未变。

## 移植依据与范围

1. [3c1441bc：Raiden II driver / Seibu sound](https://github.com/liujiny/mame2003-plus-libretro-ps3/commit/3c1441bc8567bebd810046a03593c3b421bf08d7)：引入程序 ROM 分银行、COP 保护/碰撞/DMA/排序、图层银行和滚动、精灵解密、输入布局、YM2151 与双 OKIM6295 声音及相关状态注册。
2. [938c4480：packed tile transparency / sprite latching](https://github.com/liujiny/mame2003-plus-libretro-ps3/commit/938c448045d8eeec992d5036cfff687412850da1)：处理代码为零的空精灵描述符，使用硬件锁存的 4 KiB 精灵快照，修复压缩图块高半字节未参与整块透明度分类的问题。

固定版本的原始提交补丁、相关源码、测试与下载 SHA-256 保存在 `upstream/`，清单为 `upstream/manifest.json`。

Xbox 工程使用较新的地址空间 API：将旧 MEMORY_READ/WRITE_START 迁移为 ADDRESS_MAP_START，使用 READ8/WRITE8、program_read/write_byte 和 cpunum_set_input_line 接口。
保留原 `raiden2.c` 的 XBE 分段 598；新实现放在同一编译单元的 `raiden2_playable.inc`，避免动态分段注册或 `__FILE__` 来源错位。
修复目标仅为 **`raiden2` 主集**。其余 10 个 Raiden II 克隆、Raiden DX、Zero Team 条目保持原有驱动及状态，没有把未验证版本标记成可用。
主集去掉旧 GAME_NOT_WORKING/GAME_NO_SOUND 标记，保留 GAME_IMPERFECT_GRAPHICS/GAME_IMPERFECT_SOUND，与参考驱动的成熟度一致。
参考提交中 libretro 专用 GAME_DOESNT_SERIALIZE 位和 PS3 主机测试开关不适用于本工程，没有套入 Xbox 的公共标志定义。

## 面向初代 Xbox 的优化

| 项目 | 实施与可核实收益 |
|---|---|
| 无损 4 位图像 | 全部原始颜色索引保留。解码图像由 25,427,968 字节（24.25 MiB）降至 12,713,984 字节（12.125 MiB）。 |
| 原地解码 | 使用核心原有 decodechar 和 128 字节临时图块，逐块覆盖对应 ROM 区。GFX_RAW 直接引用最终像素，消除另一整套解码图像分配。三个图像 ROM 区取消 DISPOSE，确保指针生命周期正确。 |
| 精灵 ROM 直接交错加载 | ROM_LOAD32_WORD 按原参考算法的排列直接加载，删除 8 MiB 临时副本和全区复制。文件名、大小、CRC 不变。 |
| 精灵解密 | 约 5 KiB 的临时查表代替每个字的逐位交换，并行进位算法保留原结果；运行时不保留这些表。 |
| 程序银行 | 复用已有 ROM_RELOAD 镜像，免去 256 KiB 银行备份分配。 |
| 精灵优先级 | 用 1,032 字节临时链表在一帧内分类一次，代替四次遍历全部 512 个描述符；保留各优先级内的逆序覆盖关系。 |
| 整个精灵的可见性判断 | 对完全位于画面外、包括回绕后仍不可见的对象提前退出；保留 512 像素坐标回绕、翻转、多图块扩展、裁剪和优先级。 |
| 编译 | Raiden II 单元使用展开的 /O2 优化与 /Ob2；保持分段要求，目标 /G6、SSE1，沿用全程序优化与链接设置。 |

RAM 实际节省还受 VMM 提交量与换页影响：上表中的图像容量是缓冲区字节数，不能简单相加当作每一时刻的实测物理 RAM 节省。
保留 16 MHz V30、原参考 55.47 Hz 时序、所有四个精灵优先级、背景/中景/前景/文字图层、调色板和声音配置。
没有通过跳过图层、削减精灵、缩减颜色或更改游戏时钟取得性能收益。未测量 Xbox FPS，所以“最大优化”在本交付中指已落实并验证的上述具体措施，不作最优速度保证。

## 回归验证

运行命令：

```powershell
Set-Location -LiteralPath 'D:\\ogxbox mameexotra\\MAMEoXtras-2026-Raiden2-AB'
& 'C:\\Program Files\\LibreOffice\\program\\python.exe' .\\tools\\run_raiden2_tests.py
```

主机测试使用现有 Windows Visual C++ 2010，Xbox 文件使用 XDK 自带 VC 7.1；两者输出目录分开。

| 验证项目 | 结果 |
|---|---|
| 全套图像原地解码 | 25,427,968 个像素逐个一致，所有 pen_usage 一致；raw 像素缓冲确认直接引用 ROM 区。 |
| 实际核心绘制函数 | 73,728 组压缩/非压缩绘制对比通过；含奇数跳过量、裁剪、X/Y 翻转、透明/不透明及两类调色板。 |
| 图块透明度 | 12,096 组通过；高低半字节、旋转、翻转、8/16/32 图块、行间隔等。原始 tilemap 作为反例确实失败，确认测试能发现参考提交修复的问题。 |
| 精灵与锁存 | 参考提交的回归场景通过；43,520 组优化前后绘制调用、顺序、翻转、裁剪和像素内容一致。 |
| 解密 | 两种完整 8 MiB 输入逐字节一致；FNV-1a 分别为 2b9f5865 / 8d272a0a。 |
| 进位算法 | 1,000,000 组随机 32 位输入和掩码与原逐位算法一致。 |
| 接线检查 | 0x68e/0x68f 锁存地址优先于 COP 映射、缓冲启用、ROM 交错与银行镜像、时钟设置均检查通过。 |

测试日志：`raiden2-tests.log`、`build/raiden2-tests/*.log`；生成的 C/C++ 测试程序和结果 JSON 同目录。
解密日志内的耗时来自这台 Windows 电脑的函数测试，包含主机编译器差异；不能解释为 Xbox 游戏帧率或实机加速倍数。

## vm.txt 建议

在完整的 `vm.txt` 中增加或替换这一行：

```text
raiden2 4 16 65535
```

这是 **64 MiB 初代 Xbox 的起始建议，尚未通过实际运行调优**。该行只针对本次主集及优化方式。

- `4`：单次申请**超过** 4 MiB 才使用 VMM。因此 8 MiB 精灵区会使用 VMM，而恰好 4 MiB 的背景区仍在普通内存。
- `16`：每个 4 MiB 虚拟块初始提交 16 × 64 KiB = 1 MiB RAM；8 MiB 精灵区初始共提交约 2 MiB。加载结束后会继续按剩余内存分配。
- `65535`：使用常用的分配掩码；本游戏的大块虚拟图像可获得剩余 RAM。

| 手测情况 | 替代同一行的参数 |
|---|---|
| 能加载但换页/读盘明显，尝试增加初始缓存 | `raiden2 4 32 65535` |
| 加载阶段出现物理内存不足 | `raiden2 4 8 65535` |

同名条目只保留一条，源码采用首个匹配。关闭 `ForceVMM`，否则菜单里的全局设置覆盖 vm.txt。
不要把 COMMIT 改成 64 来表示全驻留：这份源码会把 64 及以上重置为 32 对应的 2 MiB。
建议文件 `vm-raiden2-recommended.txt` 只有本游戏说明，**不能代替完整 vm.txt**。用户原 `F:\\download\\MAMEoXtras 2026\\vm.txt` 没有改动。

## 手动安装及试验

1. 在 Xbox 上复制一份完整运行目录，放入本包两个 XBE，并给该副本的完整 vm.txt 加入上述一行。
2. 保留 Media、Skins、原有运行资源和街机 ROM。包内不包含街机 ROM。
3. 运行 `default.xbe`。本版更新了驱动列表签名，首次会自动重建驱动列表；换回旧版也可能触发重建。
4. 选 **Raiden 2 (US, OG Xbox backport)**，从头开始游戏；不要用旧驱动或 PS3 核心存档验证首次启动。
5. 检查投币、开始、移动、射击/炸弹、音乐/音效、开场及第一关图层、精灵遮挡、回绕边缘和返回菜单。
6. 再单独开启 A/B 连发测试。第二动作默认可能在 X；要用 B 连发第二动作，在游戏内 Input (this game) 将 P1 Button 2 映射为 J1 B。

独立目录隔离的是程序文件；Title ID 仍为 4D414D46，配置/存档位置沿用原版，不代表 Xbox 存档完全隔离。
当前源码背景图仍可能带旧年份字样；识别本版以驱动名、文件哈希及 XBE 标题为准。

### ROM 对应关系

使用 `raiden2.zip` 主集，文件 CRC 以此表为准。此前的 `raiden.zip` 是一代，不能用于二代驱动。
同 CRC 的旧集文件可由 MAME 的 CRC 查找识别；具体打包方式仍应以实际加载检查为准。

| 文件 | 字节 | CRC32 |
|---|---:|---|
{rom_rows}

完整 SHA-1 见 `raiden2-rom-requirements.json`。本次未扫描、下载或运行用户的 Raiden II ROM。

## 下次手动编译

工程与工具链位置保持当前布局即可。已有 XDK 5849（VC 7.1）位于父目录 `toolchain/XDK`；脚本直接调用编译器，不依赖 Visual Studio IDE。

```powershell
Set-Location -LiteralPath 'D:\\ogxbox mameexotra\\MAMEoXtras-2026-Raiden2-AB'
# 增量编译、链接、生成两个 XBE 和 ZIP
.\\build_xbe.cmd --jobs 8

# 全部重新编译
.\\build_xbe.cmd --rebuild --jobs 8

# 单独复核 XBE 结构及分段哈希
& 'C:\\Program Files\\LibreOffice\\program\\python.exe' .\\verify_xbe.py `
  .\\dist\\MAMEoXtras-2026-Raiden2-AB\\default.xbe `
  .\\dist\\MAMEoXtras-2026-Raiden2-AB\\MAMEoX.xbe
```

`build_xbe.cmd` 自动使用已安装的 LibreOffice Python。需要换 Python 时先设置 `$env:MAMEOX_PYTHON` 为 Python 3 的完整路径。
新增 .inc 已纳入编译失效检测，修改 COP、解密或绘制 include 后不会误用旧对象。
最终构建日志为 `build-raiden2-final.log`；编译响应文件、每文件日志、链接 MAP、未转换 XDK XBE 均保留在 `build/`。
如果改过源码，重新运行回归脚本再执行 `tools/finalize_raiden2.py` 更新报告和最终包；该工具需要完整编译日志中的 Package 完成行。

## 输出校验

| 文件 | 字节 | SHA-256 |
|---|---:|---|
{artifact_rows}

构建汇总与旧版文件未变检查：`build/raiden2-build-summary.json`。ZIP 自身校验值见 `dist/SHA256SUMS-Raiden2.txt`。

## 修改的源码/工程文件

{changed_rows}

另更新独立目录中的 `build_xbe.py` 输出包名称、.inc 依赖检测、随包说明文件；提供可复现的移植/优化脚本及主机回归脚本。
'''
    report_path = ROOT / "编译报告_Raiden2.md"
    report_path.write_text(report, encoding="utf-8-sig")
    shutil.copy2(report_path, output / report_path.name)
    shutil.copy2(ROOT / "raiden2-rom-requirements.json", output / "raiden2-rom-requirements.json")
    archive_path = output.parent / "MAMEoXtras-2026-Raiden2-AB-XBE.zip"
    with ZipFile(archive_path, "w", ZIP_DEFLATED, compresslevel=6) as archive:
        for path in sorted(output.rglob("*")):
            if path.is_file() and path.name.lower() != "thumbs.db":
                archive.write(path, Path(output.name) / path.relative_to(output))
    with ZipFile(archive_path) as archive:
        assert archive.testzip() is None
        for entry in xbes:
            name = Path(entry["file"]).name
            assert hashlib.sha256(archive.read(output.name + "/" + name)).hexdigest() == entry["sha256"]
    archive_hash = sha(archive_path)
    (output.parent / "SHA256SUMS-Raiden2.txt").write_text(f"{archive_hash}  {archive_path.name}\n", encoding="ascii")
    summary = {"compiled_units": count, "errors": 0, "warning_count": len(warnings), "warnings": warnings,
               "xbe": [{k:v for k,v in x.items() if k != "sections"} for x in xbes],
               "archive": {"file": str(archive_path), "size": archive_path.stat().st_size, "sha256": archive_hash},
               "previous_ab_binaries_unchanged": unchanged, "autofire_source_files_unchanged": autofire_files,
               "tests": tests, "changed_source_file_count": len(changes), "report": str(report_path),
               "runtime_tested": False, "vm_recommendation": "raiden2 4 16 65535"}
    (ROOT / "build/raiden2-build-summary.json").write_text(json.dumps(summary, indent=2, ensure_ascii=False) + "\n", encoding="utf-8")
    print(json.dumps({"compiled_units": count, "warnings": len(warnings), "changes": len(changes), "archive_sha256": archive_hash, "report": str(report_path)}, ensure_ascii=True, indent=2))

if __name__ == "__main__":
    main()
