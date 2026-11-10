# GF Amiga 4Pane

4Pane 8.0, the four-pane file manager, for AmigaOS 3 (68040; the FPU when there is one, through fpuauto), on the GFOpenWX Toolkit (wxWidgets on GFOpenLayout).

4Pane is fetched at build time at the version `4pane.lock` pins, and `patches/` is applied to it; nothing third-party is kept in this repository.

## Status

4Pane 8.0 runs on AmigaOS 3 (tested 6 Nov 2026 in a 68040 AmiCradle lab): the twin-pane trees and file lists, the toolbar, the XRC dialogs and the first-run wizard. Start it with a double-click on its icon, or from a Shell in its drawer. It needs gfopenpng.datatype for its PNG pictures.

Not done yet: Amiga devices and mounting (4Pane's Linux device and mount code finds nothing here), terminal emulator and archive tools that call Unix programs.

## Building

```
STOVE=~/AmiCradle/stoves/os32-gcc16-open6 OUT=out ./build-amiga.sh
```

The first run fetches the GFOpenWX Toolkit at the commit `4pane.lock` pins and builds its SDK, then 4Pane for what is left of 840 s; run it again and it goes on from where it stopped. `out/4Pane/` gets the program, its icon, its dialogs (`rc/`), pictures (`bitmaps/`) and manual (`doc/`) when it links, and `out/4Pane-8.0.lha` holds that drawer.

| Where | What |
|---|---|
| `patches/` | 4Pane's changes for the Amiga |
| `src/` | the port's own pieces |
| `scripts/` | the 4Pane build and the link specs |

## ARexx

4Pane has an ARexx port, `4PANE.1` (`4PANE.2` for a second copy), from the GFOpenWX Toolkit's `wxARexxPort` on arexx.class (`src/amiga_rexx.cpp`, patch 0006).

| Command | Does |
| --- | --- |
| `GOTO PATH/A` | the active pane shows PATH (a drawer, or a file's drawer with the file selected) |
| `GETPATH` | RESULT: the active pane's selected path |
| `GETSELECTED` | RESULT: every selected path in the active pane, each in double quotes |
| `REFRESH` | the active pane reads its drawer again |
| `QUIT`, `ACTIVATE`, `DEACTIVATE`, `VERSION`, `HELP`, `FAULT` | the toolkit's standard commands |

RC 10 when there is no active pane.

```
/* 4Pane: show Work:Projects, then say what is selected */
OPTIONS RESULTS
ADDRESS '4PANE.1'
'GOTO Work:Projects'
'GETSELECTED'
SAY result
```

`Rexx/Here.rexx` (in the package's Rexx drawer) is an example. `tests/arexx.rexx` (in the tests drawer beside it) checks the whole port: `rx tests/arexx.rexx` with the program running; it QUITs the program at the end.

Return codes, as OS 3.2's arexx.class: 0 OK, 5 warning, 10 error, 20 unknown command or failure.

## Licence

4Pane is GPL v3 (some parts wxWindows Licence); the patches and build files here are GPL v3.

Designed, tested and delivered by The AmiCradle Team.
Copyright (c) 2026 AmiCradle Team
