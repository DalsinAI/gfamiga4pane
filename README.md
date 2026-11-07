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

## Licence

4Pane is GPL v3 (some parts wxWindows Licence); the patches and build files here are GPL v3.

Designed, tested and delivered by The AmiCradle Team.
Copyright (c) 2026 AmiCradle Team
