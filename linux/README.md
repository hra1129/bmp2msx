# bmp2msx-cli — BMP2MSX for Linux

A headless (command-line) Linux build of BMP2MSX. It runs the same
conversion core as the Windows application — `../source/Convert.cpp`,
`bsave.cpp`, `utils.cpp` and `calc.cpp` are compiled **unmodified** —
so, given the same settings and the same input pixels, the output files
are byte-identical to the Windows version's.

The Windows GUI is not ported; this directory adds only the Linux
support code:

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

## What is intentionally not included

These GUI-only features have no headless counterpart and are excluded:

- preview display and preview BMP export, drag & drop, clipboard input
- the setting/calibration/spit (eyedropper) dialogs — calibration
  *data* (`BMP2MSX_CUSTOM.BIN`) is honored, its editor is not
- the script `SETTING` command opens no dialog; it prints the current
  settings instead (the script `MSG` command likewise prints to stdout)

Exit status: 0 = success, 1 = usage/startup error, 2 = conversion or
script error.
