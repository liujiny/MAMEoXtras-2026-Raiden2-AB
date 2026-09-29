# RXDK build branch

This branch ports MAMEoXtras 2026 Raiden2 + AutoFire to the open RXDK toolchain.

## Verified toolchain

The current port was verified on Windows x64 with:

- RXDK-VS20XX CLI / SDK 1.3.2
- Xbox LLVM Clang 24.0.0git
- target `i686-pc-windows-gnu`
- CPU tuning `pentium3`

The last full validation compiled 1956/1956 translation units, linked both XBE
targets with zero linker warnings, validated XBE section hashes/entry/imports,
and packaged the result.  It has **not** been validated on real Xbox hardware,
with ROMs, or for FPS/performance.

## One-time setup

Requirements: Windows x64, Git, Python 3, .NET 8 runtime, and internet access.

From the repository root:

```powershell
python .\build\rxdk\setup_rxdk.py
```

The setup script downloads the RXDK-VS20XX 1.3.2 VSIX from Team-Resurgent,
verifies its published SHA-256, and stages RXDK SDK, host tools, samples and LLVM
under `build/rxdk/toolchain`.  All downloaded files are ignored by Git.

To check an existing setup without downloading:

```powershell
python .\build\rxdk\setup_rxdk.py --check
```

## Full build

```powershell
python -u -B .\build\rxdk\rebuild_release.py --jobs 8
```

The build refreshes an isolated source mirror in `build/rxdk/source`, compiles
resources, performs an 8-way full compile sweep, archives libraries, links both
programs through the RXDK CLI, runs the host-side VMM/ABI checks, validates the
XBE files, and creates a fresh package under `dist`.

Progress is written to `build/rxdk/BUILD_PROGRESS.json`; detailed compiler and
linker logs stay under `build/rxdk/logs` and are ignored by Git.

## Important notes

- The checked-out source is the source of truth.  The mirror is recreated before
  every full build and is disposable.
- No Microsoft XDK 5849 object, library or XBE is used by this RXDK flow.
- VMM demand paging remains enabled.  The port uses an x86 GNU-ABI SEH
  registration layer instead of deleting the original fault recovery.
- `RxdkDsoundMath.c` supplies real stdcall-to-cdecl bridges for the RXDK
  DirectSound math calls; linker name auto-fixups are treated as an error.
- Generated XBE/ZIP files are homebrew outputs, not Microsoft retail signatures.
