"""Build the supplied VS 2003 Xbox projects using the extracted XDK 5849.

Uses the original Release|Xbox file lists and compiler settings. No IDE is needed.
Run with Python 3 on Windows. Compiler output and response files are kept in build/.
"""
from __future__ import annotations

import argparse
import concurrent.futures
import hashlib
import json
import os
from pathlib import Path
import re
import shutil
import subprocess
import sys
import tarfile
import xml.etree.ElementTree as ET
from zipfile import ZIP_DEFLATED, ZipFile
from verify_xbe import patch_retail

ROOT = Path(__file__).resolve().parent
SRC = ROOT / "MAMEoXtras 2026 Src"
SDK = ROOT.parent / "toolchain/XDK"
BIN = SDK / "xbox/bin"
VC = BIN / "vc71"
BUILD = ROOT / "build"
PROJECTS = {
    "libsmb": SRC / "libsmb++/libsmb.vcproj",
    "MAME": SRC / "MAME/MAME.vcproj",
    "MAMEoX": SRC / "MAMEoX/MAMEoX.vcproj",
    "MAMEoXLauncher": SRC / "MAMEoXLauncher/MAMEoXLauncher.vcproj",
}
ENV = os.environ.copy()
ENV.update(XDK=str(SDK), INCLUDE=str(SDK / "xbox/include"), LIB=str(SDK / "xbox/lib"))
ENV["PATH"] = os.pathsep.join([str(VC), str(BIN), ENV.get("PATH", "")])
ENV.pop("CL", None)
ENV.pop("_CL_", None)


def run(args, cwd, log):
    log.parent.mkdir(parents=True, exist_ok=True)
    with log.open("wb") as output:
        proc = subprocess.run([str(a) for a in args], cwd=cwd, env=ENV,
                              stdout=output, stderr=subprocess.STDOUT)
    if proc.returncode:
        detail = log.read_text(encoding="mbcs", errors="replace")
        raise RuntimeError(f"Exit {proc.returncode}: {args[0]}\n{detail[-10000:]}\nLog: {log}")


def rsp(path, args):
    path.parent.mkdir(parents=True, exist_ok=True)
    # Windows command-line quoting, also understood by the VC 7.1 response parser.
    path.write_text("\n".join(subprocess.list2cmdline([str(a)]) for a in args) + "\n",
                    encoding="mbcs")
    return "@" + str(path)


def prepare():
    for path in [VC / "cl.exe", VC / "link.exe", BIN / "imagebld.exe", BIN / "bundler.exe"]:
        if not path.is_file():
            raise RuntimeError(f"Missing XDK tool: {path}")
    # The source distribution contains only the zlib project and an old public header.
    # Restore the compatible upstream 1.1.4 source without replacing supplied files.
    if not (SRC / "ZLIB/zconf.h").exists():
        with tarfile.open(ROOT.parent / ".downloads/zlib-1.1.4.tar.gz") as archive:
            for item in archive.getmembers():
                parts = Path(item.name).parts
                if len(parts) != 2 or not item.isfile():
                    continue
                if Path(parts[-1]).suffix not in (".c", ".h") and parts[-1] not in ("README", "ChangeLog"):
                    continue
                dest = SRC / "ZLIB" / parts[-1]
                if not dest.exists():
                    dest.write_bytes(archive.extractfile(item).read())


def resources():
    media = SRC / "SharedResources/MediaFiles"
    (SRC / "SharedResources/Includes").mkdir(exist_ok=True)
    latest_input = max(path.stat().st_mtime_ns for path in media.iterdir()
                       if path.suffix.lower() in (".rdf", ".abc", ".tga", ".bmp", ".dds", ".png"))
    for resource in sorted(media.glob("*.rdf")):
        target = SRC / "Media" / (resource.stem + ".xpr")
        header = SRC / "SharedResources/Includes" / (resource.stem + ".h")
        if (target.exists() and header.exists()
                and min(target.stat().st_mtime_ns, header.stat().st_mtime_ns) >= latest_input):
            continue
        print(f"Bundling {resource.name}", flush=True)
        run([BIN / "bundler.exe", resource.name], media, BUILD / "logs" / (resource.stem + ".log"))
        if not target.exists() or not header.exists():
            raise RuntimeError(f"Resource bundler did not produce {target} and {header}")


def split_list(value):
    return [part.strip().strip('"') for part in re.split(r"[;,]", value) if part.strip()]


def compiler_flags(settings, directory):
    flags = ["/nologo", "/c", "/W3", "/ML", "/Gd", "/Y-"]
    optimization = settings.get("Optimization")
    if optimization == "2" and settings.get("EnableFunctionLevelLinking") == "FALSE":
        # VC 7.1 has no /Gy- switch. Expand /O2 without its implicit /Gy.
        flags += ["/Og", "/Oi", "/Ot", "/Oy", "/Ob2", "/Gs"]
    elif optimization in {"0", "1", "2", "3"}:
        flags += [{"0": "/Od", "1": "/O1", "2": "/O2", "3": "/Ox"}[optimization]]
    for key, option in [
        ("GlobalOptimizations", "/Og"), ("EnableIntrinsicFunctions", "/Oi"),
        ("OmitFramePointers", "/Oy"), ("EnableFiberSafeOptimizations", "/GT"),
        ("StringPooling", "/GF"), ("EnableFunctionLevelLinking", "/Gy"),
        ("BufferSecurityCheck", "/GS"), ("ForceConformanceInForLoopScope", "/Zc:forScope"),
    ]:
        if key in settings:
            enabled = settings[key].upper() == "TRUE"
            # These switches default off in VC 7.1 and do not accept a minus.
            if option in ("/Gy", "/GS") and not enabled:
                continue
            flags.append(option + ("" if enabled else "-"))
    if "InlineFunctionExpansion" in settings:
        flags.append("/Ob" + settings["InlineFunctionExpansion"])
    if settings.get("FavorSizeOrSpeed") == "1":
        flags.append("/Ot")
    if settings.get("FavorSizeOrSpeed") == "2":
        flags.append("/Os")
    processor = settings.get("OptimizeForProcessor")
    if processor in ("0", "1", "2", "3"):
        flags.append({"0": "/GB", "1": "/G5", "2": "/G6", "3": "/G7"}[processor])
    if settings.get("EnableEnhancedInstructionSet") == "1":
        flags.append("/arch:SSE")
    if settings.get("ExceptionHandling", "FALSE").upper() == "TRUE":
        flags.append("/EHsc")
    flags += settings.get("AdditionalOptions", "").split()
    flags += ["/D" + define for define in split_list(settings.get("PreprocessorDefinitions", ""))]
    include_dirs = split_list(settings.get("AdditionalIncludeDirectories", ""))
    # zconf.h is absent from the source package; both original zlib headers use it.
    include_dirs += [str(SRC / "ZLIB"), str(SRC / "SharedResources/Includes")]
    for include in include_dirs:
        path = Path(include.replace("$(XDK)", str(SDK)))
        if not path.is_absolute():
            path = directory / path
        flags.append("/I" + str(path.resolve()))
    flags += ["/FI" + header for header in split_list(settings.get("ForcedIncludeFiles", ""))]
    return flags


def sources(name):
    project = PROJECTS[name]
    tree = ET.parse(project)
    config = tree.find('.//Configurations/Configuration[@Name="Release|Xbox"]')
    base = config.find('Tool[@Name="VCCLCompilerTool"]').attrib
    base = dict(base)
    if config.get("CharacterSet") == "2":
        base["PreprocessorDefinitions"] = base.get("PreprocessorDefinitions", "") + ";_MBCS"
    result = []
    for entry in tree.findall(".//File"):
        local = entry.find('FileConfiguration[@Name="Release|Xbox"]')
        if local is not None and local.get("ExcludedFromBuild", "").upper() == "TRUE":
            continue
        relative = entry.get("RelativePath").replace("$(XDK)", str(SDK))
        path = Path(relative)
        if path.suffix.lower() not in (".c", ".cpp"):
            continue
        if not path.is_absolute():
            path = project.parent / path
        path = path.resolve()
        if not path.is_file():
            raise RuntimeError(f"Missing active source: {path}")
        settings = dict(base)
        if local is not None:
            override = local.find('Tool[@Name="VCCLCompilerTool"]')
            if override is not None:
                settings.update(override.attrib)
        key = path.stem + "-" + hashlib.sha1(str(path).encode()).hexdigest()[:10]
        obj = BUILD / "obj" / name / (key + ".obj")
        # Compile using original relative paths to preserve __FILE__ driver names.
        source_arg = os.path.relpath(path, project.parent)
        args = compiler_flags(settings, project.parent) + ["/Fo" + str(obj), source_arg]
        result.append((project.parent, path, obj, args))
    return result


def compile_one(job, header_stamp, rebuild):
    directory, source, obj, args = job
    stamp_path = obj.with_suffix(".json")
    stamp = json.dumps([args, source.stat().st_mtime_ns, header_stamp])
    if not rebuild and obj.exists() and stamp_path.exists() and stamp_path.read_text() == stamp:
        return False
    obj.parent.mkdir(parents=True, exist_ok=True)
    response = rsp(obj.with_suffix(".rsp"), args)
    run([VC / "cl.exe", response], directory, obj.with_suffix(".log"))
    stamp_path.write_text(stamp)
    return True


def compile_projects(names, jobs, match, rebuild=False):
    # The Raiden II port includes production code from .inc files.
    header_stamp = max(p.stat().st_mtime_ns for pattern in ("*.h", "*.inc")
                       for p in SRC.rglob(pattern))
    for name in names:
        units = sources(name)
        if match:
            units = [unit for unit in units if match.lower() in str(unit[1]).lower()]
        print(f"Compiling {name}: {len(units)} files, {jobs} workers", flush=True)
        failures = []
        changed = 0
        with concurrent.futures.ThreadPoolExecutor(max_workers=jobs) as pool:
            pending = {pool.submit(compile_one, unit, header_stamp, rebuild): unit for unit in units}
            for count, task in enumerate(concurrent.futures.as_completed(pending), 1):
                try:
                    changed += task.result()
                except Exception as error:
                    failures.append(str(error))
                    print(str(error), flush=True)
                if count % 100 == 0 or count == len(units):
                    print(f"{name}: {count}/{len(units)}, compiled {changed}, errors {len(failures)}", flush=True)
        if failures:
            raise RuntimeError(f"{name}: {len(failures)} compile errors; see build/obj/{name}/*.log")


def link_projects(names):
    executables = [name for name in names if name in ("MAMEoX", "MAMEoXLauncher")]
    libraries = ["libsmb"] if executables else [name for name in names if name in ("libsmb", "MAME")]
    if "MAMEoX" in executables:
        libraries.append("MAME")
    for name in libraries:
        output = BUILD / (name + ".lib")
        objects = [unit[2] for unit in sources(name)]
        if any(not obj.exists() for obj in objects):
            raise RuntimeError(f"Compile {name} before linking")
        args = ["/nologo", "/LTCG", "/OUT:" + str(output)] + [str(obj) for obj in objects]
        print(f"Archiving {name}", flush=True)
        run([VC / "lib.exe", rsp(BUILD / (name + "-lib.rsp"), args)], SRC, BUILD / "logs" / (name + "-lib.log"))
    for name in executables:
        project = PROJECTS[name]
        tree = ET.parse(project)
        config = tree.find('.//Configurations/Configuration[@Name="Release|Xbox"]')
        settings = config.find('Tool[@Name="VCLinkerTool"]').attrib
        args = ["/nologo", "/MACHINE:I386", "/SUBSYSTEM:XBOX", "/FIXED:NO", "/LTCG",
                "/INCREMENTAL:NO", "/OPT:REF", "/OPT:ICF", "/LIBPATH:" + str(BUILD),
                "/OUT:" + str(BUILD / (name + ".exe")), "/MAP:" + str(BUILD / (name + ".map"))]
        args += [str(unit[2]) for unit in sources(name)]
        args += settings["AdditionalDependencies"].split()
        print(f"Linking {name} (whole program optimization)", flush=True)
        run([VC / "link.exe", rsp(BUILD / (name + "-link.rsp"), args)], project.parent,
            BUILD / "logs" / (name + "-link.log"))


def images():
    output = ROOT / "dist/MAMEoXtras-2026-Raiden2-AB"
    output.mkdir(parents=True, exist_ok=True)
    verified = []
    for name, target, title, stack in [
        ("MAMEoX", "MAMEoX.xbe", "MAMEoX Util (Don't Run)", "0xc0000"),
        ("MAMEoXLauncher", "default.xbe", "MAMEoXtras 2026 Raiden2 + AB", "0x10000"),
    ]:
        args = ["/NOLOGO", "/NOLIBWARN", "/FORMATUD", "/TESTID:0x4D414D46",
                "/TESTNAME:" + title, "/STACK:" + stack,
                "/IN:" + str(BUILD / (name + ".exe")), "/OUT:" + str(output / target),
                "/TITLEIMAGE:" + str(SRC / "Media/mox-icon.xpr"),
                "/DEFAULTSAVEIMAGE:" + str(SRC / "Media/mox-icon.xpr")]
        # Both driver and CPU sections must load at boot in this 2026 source:
        # xbox_Main reads drivers[] and cpuintrf_init calls every CPU's get_info
        # before cpuexec releases unused CPU sections. The older Sectionize.pl
        # script's CPU /NOPRELOAD switches are incompatible with this code.
        if name == "MAMEoX":
            # Original UDCluster=3 is eXboxUDC_64K in VCProjectEngine's type library.
            args.append("/UDCLUSTER:65536")
        print(f"Building {target}", flush=True)
        run([BIN / "imagebld.exe", rsp(BUILD / (name + "-image.rsp"), args)], SRC,
            BUILD / "logs" / (name + "-image.log"))
        raw_dir = BUILD / "xdk-xbe"
        raw_dir.mkdir(exist_ok=True)
        shutil.copy2(output / target, raw_dir / target)
        verified.append(patch_retail(output / target))
    for directory in ("Media", "Skins", "Docs"):
        shutil.copytree(SRC / directory, output / directory, dirs_exist_ok=True)
    for filename in ("README_AUTOFIRE.txt", "AutoFire-defaults.ini.example"):
        shutil.copy2(ROOT / filename, output / filename)
    for filename in ("README_RAIDEN2.txt", "vm-raiden2-recommended.txt"):
        shutil.copy2(ROOT / filename, output / filename)
    (BUILD / "xbe-verification.json").write_text(json.dumps(verified, indent=2) + "\n", encoding="utf-8")
    (output / "SHA256SUMS.txt").write_text("".join(
        f"{entry['sha256']}  {Path(entry['file']).name}\n" for entry in verified), encoding="ascii")
    (output / "README_BUILD.txt").write_text(
        "MAMEoXtras 2026 / Original Xbox / Release build\n\n"
        "启动文件：default.xbe；模拟器核心：MAMEoX.xbe。两个文件必须放在同一目录。\n"
        "将这两个 XBE 放入完整的 MAMEoXtras 2026 安装目录，启动 default.xbe。\n"
        "请保留完整发行版的 vm.txt、配置及运行资源。\n"
        "本包附带源码中的 Media、Skins、Docs，并非完整发行包；源码压缩包没有 vm.txt。\n"
        "XBE 已通过结构、分段哈希和内核导入校验，尚未进行真机运行测试。\n"
        "Retail 编码用于支持自制程序的初代 Xbox，不是微软官方零售签名。\n"
        "文件校验值见 SHA256SUMS.txt。\n", encoding="utf-8-sig")
    archive_path = output.parent / "MAMEoXtras-2026-Raiden2-AB-XBE.zip"
    with ZipFile(archive_path, "w", compression=ZIP_DEFLATED, compresslevel=6) as archive:
        for path in sorted(output.rglob("*")):
            if path.is_file() and path.name.lower() != "thumbs.db":
                archive.write(path, Path(output.name) / path.relative_to(output))
    print(f"Outputs: {output}", flush=True)
    print(f"Package: {archive_path}", flush=True)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--stage", choices=["all", "resources", "compile", "link", "images"], default="all")
    parser.add_argument("--projects", nargs="+", choices=list(PROJECTS), default=list(PROJECTS))
    parser.add_argument("--jobs", type=int, default=min(8, os.cpu_count() or 2))
    parser.add_argument("--match", help="Compile only source paths containing this substring")
    parser.add_argument("--rebuild", action="store_true", help="Recompile all selected files without deleting anything")
    options = parser.parse_args()
    BUILD.mkdir(exist_ok=True)
    prepare()
    if options.stage in ("all", "resources"):
        resources()
    if options.stage in ("all", "compile"):
        compile_projects(options.projects, options.jobs, options.match, options.rebuild)
    if options.stage in ("all", "link"):
        link_projects(options.projects)
    if options.stage in ("all", "images"):
        images()


if __name__ == "__main__":
    try:
        main()
    except Exception as error:
        print(error, file=sys.stderr, flush=True)
        sys.exit(1)
