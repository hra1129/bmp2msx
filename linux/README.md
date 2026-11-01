# BMP2MSX for Linux (CLI + wxWidgets GUI)

A Linux build of BMP2MSX: a headless command-line front end
(`bmp2msx-cli`) and a wxWidgets GUI (`bmp2msx-wx`, see "GUI" below).
Both run the same conversion core as the Windows application —
`../source/Convert.cpp`, `bsave.cpp`, `utils.cpp` and `calc.cpp` are
compiled **unmodified** — so, given the same settings and the same
input pixels, the output files are byte-identical to the Windows
version's.

This directory adds only the Linux support code, plus a wxWidgets GUI
(`wx/`, see "GUI" below) that is written portably so it can also serve
as the basis of a future Windows GUI:

| Path | Contents |
|---|---|
| `src/main_cli.cpp` | command-line front end |
| `src/convert_run.cpp` | conversion driver (headless counterpart of the GUI's `Converter()`/`CreatePal()`) |
| `src/script.cpp` | script interpreter (same language as the GUI script window) |
| `src/image_load.cpp` | BMP/PNG/JPEG loading (replaces the WIC loader) |
| `src/fileuty_stdio.cpp` | stdio implementation of the `fileuty.h` interface |
| `src/platform_linux.cpp` | stubs for the Win32 calls the core references |
| `winshim/windows.h` | minimal `<windows.h>` replacement for the core |
| `third_party/stb_image.h` | PNG/JPEG decoder (vendored, see below) |
| `wx/` | wxWidgets front end (`bmp2msx-wx`, see below) |

## Build

```sh
make            # or: make -j
./bmp2msx-cli --help
```

Requires only g++ (C++17). Objects are placed under `obj/` with
dependency files, so incremental/parallel builds work. `make clean`
removes `obj/` and the binary. The Windows solution in `../source` is
not touched by this Makefile. (The Makefile generates lower-case
header-name symlinks under `obj/inc/` because some Windows sources
include headers with different letter case than the file names.)

## Usage

Convert one image:

```sh
./bmp2msx-cli [options] photo.bmp
```

Run a script (same language as the GUI script window, including the
Japanese command names; the Phase 3 EPAL modes can be selected with
`SCREEN 8P` / `SCREEN 8P256`):

```sh
./bmp2msx-cli --script convert.txt [--input photo.bmp]
```

See `--help` for all options. Options cover every conversion-relevant
setting of the GUI: screen mode (including the 256-line variants and
SCREEN8 EPAL), palette output (`bsave`/`pl`/`csv`/`none`), BAS loader
output, interlace, 192-line mode, resize/resample/aspect mode, error
diffusion and its coefficients, dither pattern and add mode, cut-off
error, palette selection algorithm, SCREEN10/12 algorithm, color
recalculation, reproduction accuracy, fixed/auto palette, palette
entries and per-entry mode, palette sort, non-zero, tiles, force-zero
(color/matrix) and background color.

Settings start from the application's own defaults. The raw
`BMP2MSX.B2M` config file is *not* read automatically; load one
explicitly with `--load-setting file.B2M` (command-line options then
override it) or from a script with `LOADSETTING`. If
`BMP2MSX_CUSTOM.BIN` (calibration tables) is present in the current
directory it is loaded at startup, as the Windows application does.

### Output file names

As in the GUI's automatic naming, outputs are derived from the input
file name (`photo.bmp` → `photo.SC5`, `photo.SRP`, ...), written beside
the input or into `--output-dir`. The GUI asks for the palette file
name when the palette output is PL/CSV; the CLI uses
`<name>.PL<digit>` (PL5 for the SCREEN5/6 family, PL7 for SCREEN7/10,
PL2/PL3 for SCREEN2/3) and `<name>.CSV`.

### Script text encoding

Scripts and file names are UTF-8. A script file that is not valid
UTF-8 is assumed to be Shift_JIS (the encoding older scripts were
written in on Windows) and is converted automatically.

## Image input

- BMP: an uncompressed 24bpp BMP with the standard header takes the
  same fast path as on Windows. Other BMP depths (1/4/8/16/32bpp,
  including top-down files) are normalized to 24bpp first — the role
  WIC plays on Windows.
- PNG / JPEG: decoded with stb_image (`third_party/stb_image.h`,
  v2.30, dual-licensed public domain / MIT; the license text is in the
  header itself). PNG is lossless, so PNG input gives the same results
  as the equivalent BMP. JPEG decompression is not bit-exact across
  decoders, so JPEG results can differ from the Windows (WIC) build by
  rounding in a few pixels.

## GUI (wxWidgets)

`wx/` contains a wxWidgets front end over the same conversion core,
the Linux counterpart of the Windows GUI:

```sh
make gui        # builds ./bmp2msx-wx (the CLI target is unaffected)
./bmp2msx-wx
```

Requirements: a C++17 compiler plus the wxWidgets development files
(`wx-config` must be on the PATH).  On Ubuntu 24.04:

```sh
sudo apt install libwxgtk3.2-dev    # wxWidgets 3.2.4 (wxGTK)
```

(packages installed with apt do not survive a VM/image reset; the CLI
needs none of this.)

Coverage, mirroring the Windows GUI:

- Main window with the preview list (up to 50 previews): open an image
  and it is converted with the current default settings; Right/Left
  navigate, Space toggles source/result, F7/F8 zoom, Delete clears,
  Ctrl+A clears all; "Show original" (Ctrl+O) opens the original-image
  window with the eyedropper; the settings text under the preview can
  be toggled from the Settings menu; clicking the preview re-opens the
  settings for re-conversion.
- File output identical to the CLI (same core, same save functions):
  save / save-as, save as BMP (24-bit), automatic file names, the
  PL/CSV palette-file prompts.
- Settings dialog with every conversion setting (screen mode incl.
  the SCREEN8 EPAL modes, output target, palette editor with
  load/save, dither/error diffusion, resize, force-zero, ...), usable
  both as the default settings and for re-conversion; settings files
  (BMP2MSX.B2M) load/save through the core functions; the defaults are
  read at startup and saved on exit, next to the executable, like the
  Windows app.  The calibration file BMP2MSX_CUSTOM.BIN is honored
  exactly as in the CLI.
- Script window: edit and run scripts in the existing script language;
  output goes to the log pane.
- Clipboard: Edit > Convert clipboard image pastes an image and
  converts it.

Differences from the Windows GUI (deliberate, small):

- The color-calibration *editor* dialog is not implemented; the
  calibration data file is still loaded and applied.
- In the script window, the `SETTING` command prints the settings to
  the log instead of opening a dialog, and `MSG` prints to the log.
- "Save as BMP" renders through the portable preview renderer (the
  core's `cnvSaveBmpFile()` needs GDI); the source view is scaled onto
  the standard 512x424 preview canvas.

Test hooks (used by the automated checks):

```sh
./bmp2msx-wx --smoke-convert <image> <outdir>   # headless EPAL conversion
./bmp2msx-wx --snapshot <image> <out.png>       # main-window screenshot
./bmp2msx-wx --snapshot-settings <out.png>      # settings-dialog screenshot
```

### Portability / future Windows build

The GUI sources in `wx/` deliberately use only portable wxWidgets and
standard C++: no GTK-, X11- or POSIX-specific calls, no Linux path or
encoding assumptions (file dialogs, the executable-directory settings
file and the clipboard all go through wx).  The single platform seam
is `wx/capture_log.cpp` (redirecting stdout/stderr into the script
log), which is clearly marked and already sketches the `_WIN32`
branch.

A future Windows build of this same GUI would need:

- wxWidgets binaries for Windows (MSVC), with `wx-config` replaced by
  the corresponding include/library settings;
- linking the same conversion core from `../source` — on Windows the
  core builds natively, so `linux/winshim/`, `src/fileuty_stdio.cpp`
  and `src/platform_linux.cpp` are replaced by the Win32 layer, while
  `src/convert_run.cpp`, `src/script.cpp` (the script engine is
  platform-independent) and the `wx/` sources are reused as-is;
  `src/image_load.cpp` would use WIC again instead of stb_image.

## What is intentionally not included (CLI)

These GUI-only features have no headless counterpart and are excluded
from the CLI:

- preview display and preview BMP export, drag & drop, clipboard input
- the setting/calibration/spit (eyedropper) dialogs — calibration
  *data* (`BMP2MSX_CUSTOM.BIN`) is honored, its editor is not
- the script `SETTING` command opens no dialog; it prints the current
  settings instead (the script `MSG` command likewise prints to stdout)

Exit status: 0 = success, 1 = usage/startup error, 2 = conversion or
script error.
