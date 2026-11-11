#!/bin/sh
# Fetches 4Pane at the version 4pane.lock pins, applies patches/, and builds
# it against gfopenwx's SDK: 4Pane's own sources, its bzip2 copy and xz's
# liblzma (built here once, linked in: patch 0009). The
# build tree is kept, so a build that runs out of time goes on next time
# (BUDGET seconds of make per run). OUT gets 4Pane/: the program, its
# dialogs (rc), pictures (bitmaps) and manual (doc).
#   TOOLCHAIN=.../prefix WX_SDK=... [FPUSPECS=fpuauto.specs FPULIB=dir] WORK=build OUT=out scripts/build-4pane.sh
# Copyright (c) 2026 AmiCradle Team
set -eu
HERE=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
. "$HERE/4pane.lock"
WORK=${WORK:-$HERE/build}; OUT=${OUT:-$WORK/out}; JOBS=${JOBS:-12}; BUDGET=${BUDGET:-700}
PATH="$TOOLCHAIN/bin:$PATH"
SRC="$WORK/4pane"; B="$WORK/obj"; D="$WORK/dl"
mkdir -p "$OUT" "$D"

if [ ! -f "$D/4pane.tar.gz" ] || ! echo "$FP_SHA256  $D/4pane.tar.gz" | sha256sum -c - >/dev/null 2>&1; then
    curl -fsSL -o "$D/4pane.tar.gz.part" "$FP_URL"
    echo "$FP_SHA256  $D/4pane.tar.gz.part" | sha256sum -c - >/dev/null || { echo "sha256 mismatch: $FP_URL"; exit 1; }
    mv "$D/4pane.tar.gz.part" "$D/4pane.tar.gz"
fi

# liblzma: xz at 4pane.lock's version, configured for the stove, built once into
# $WORK/xz (the library alone: no xz programs, threads or translations)
XZ="$WORK/xz"
if [ "$(cat "$XZ/.sha256" 2>/dev/null)" != "$XZ_SHA256" ] || [ ! -f "$XZ/inst/lib/liblzma.a" ]; then
    if ! echo "$XZ_SHA256  $D/xz.tar.gz" | sha256sum -c - >/dev/null 2>&1; then
        curl -fsSL -o "$D/xz.tar.gz.part" "$XZ_URL"
        echo "$XZ_SHA256  $D/xz.tar.gz.part" | sha256sum -c - >/dev/null || { echo "sha256 mismatch: $XZ_URL"; exit 1; }
        mv "$D/xz.tar.gz.part" "$D/xz.tar.gz"
    fi
    rm -rf "$XZ"; mkdir -p "$XZ/src" "$XZ/b"
    tar xzf "$D/xz.tar.gz" -C "$XZ/src" --strip-components=1
    ( cd "$XZ/b" && CC=m68k-amigaos-gcc CFLAGS="-noixemul -m68040 -O2" ../src/configure --host=m68k-amigaos --prefix="$XZ/inst" \
        --disable-shared --enable-static --disable-threads --disable-xz --disable-xzdec --disable-lzmadec --disable-lzmainfo \
        --disable-lzma-links --disable-scripts --disable-doc --disable-nls --disable-microlzma --disable-lzip-decoder \
        --enable-encoders=lzma1,lzma2 --enable-decoders=lzma1,lzma2,x86,delta --enable-checks=crc32,crc64,sha256 \
        --disable-sandbox --disable-clmul-crc --disable-arm64-crc32 > conf.log 2>&1 \
      && nice -n 19 make -j"$JOBS" -C src/liblzma > make.log 2>&1 && make -C src/liblzma install > inst.log 2>&1 ) \
      || { echo "liblzma did not build: see $XZ/b"; exit 1; }
    echo "$XZ_SHA256" > "$XZ/.sha256"
fi

# EXTRA_PATCHES: a directory of lab-only patches (diagnostics), applied last
EXTRA=""; [ -n "${EXTRA_PATCHES:-}" ] && EXTRA=$(ls "$EXTRA_PATCHES"/*.patch 2>/dev/null || true)
PSUM=$(cat "$HERE"/patches/*.patch $EXTRA 2>/dev/null | sha256sum | cut -c1-16)
if [ "$(cat "$SRC/.patched" 2>/dev/null)" != "$FP_SHA256-$PSUM" ]; then
    rm -rf "$SRC" "$B"; mkdir -p "$SRC"
    tar xzf "$D/4pane.tar.gz" -C "$SRC" --strip-components=2
    for p in "$HERE"/patches/*.patch $EXTRA; do
        [ -f "$p" ] || continue
        patch -s -d "$SRC" -p1 < "$p" || { echo "patch failed: $p"; exit 1; }
    done
    echo "$FP_SHA256-$PSUM" > "$SRC/.patched"
fi
mkdir -p "$B"

W="$WX_SDK/bin/wx-config"
LATE=""
if m68k-amigaos-gcc -fno-late-combine-instructions -S -o /dev/null -x c /dev/null 2>/dev/null; then LATE="-fno-late-combine-instructions"; fi
CF="-noixemul -m68040 -O2 $LATE"
DEFS="-DAMIGA_STATIC_LZMA -DLZMA_API_STATIC -I$XZ/inst/include -DNDEBUG -DFP_VERSION_STR=\\\"$FP_VERSION\\\""

# 4Pane's sources (Makefile.am's list) and the port's own (src/)
CXXSRC="Accelerators Archive ArchiveStream Bookmarks Configure Devices Dup ExecuteInDialog Filetypes Misc Mounts MyDirs MyDragImage MyFiles MyFrame MyGenericDirCtrl MyNotebook MyTreeCtrl Otherstreams Redo Tools bzipstream"
BZ="blocksort bzlib compress crctable decompress huffman randtable"
M="$B/Makefile"
{
    echo "CXXF = $CF -std=gnu++17 $($W --cxxflags) -include $HERE/src/amiga4pane.h $DEFS -I$SRC -I$HERE/src/include -Wno-deprecated-declarations"
    echo "CF = $CF -DNDEBUG -I$SRC/sdk/bzip"
    OBJS=""
    for s in $CXXSRC; do
        OBJS="$OBJS $s.o"
        printf '%s.o: %s\n\tm68k-amigaos-g++ $(CXXF) -MMD -c %s -o $@\n' "$s" "$SRC/$s.cpp" "$SRC/$s.cpp"
    done
    for s in "$HERE"/src/*.cpp "$HERE"/src/*.c; do
        [ -f "$s" ] || continue
        o="port_$(basename "$s" | sed 's/\.[a-z]*$//').o"
        OBJS="$OBJS $o"
        case "$s" in
            *.cpp) printf '%s: %s\n\tm68k-amigaos-g++ $(CXXF) -MMD -c %s -o $@\n' "$o" "$s" "$s" ;;
            *) printf '%s: %s\n\tm68k-amigaos-gcc $(CF) -I%s -MMD -c %s -o $@\n' "$o" "$s" "$HERE/src" "$s" ;;
        esac
    done
    for s in $BZ; do
        OBJS="$OBJS bz_$s.o"
        printf 'bz_%s.o: %s\n\tm68k-amigaos-gcc $(CF) -c %s -o $@\n' "$s" "$SRC/sdk/bzip/$s.c" "$SRC/sdk/bzip/$s.c"
    done
    echo "OBJS =$OBJS"
    echo "all: \$(OBJS)"
    echo "-include *.d"
} > "$M"
rc=0
(cd "$B" && timeout "$BUDGET" nice -n 19 make -k -j"$JOBS" all) > "$WORK.make.log" 2>&1 || rc=$?
grep -E 'error|Error' "$WORK.make.log" | grep -v '^make.*Waiting' | head -60
if [ "$rc" = 124 ]; then echo "make ran out of time: run again to go on"; exit 0; fi
[ "$rc" = 0 ] || { echo "make failed ($rc)"; exit 1; }

# link, and the program's drawer: 4Pane, rc/, bitmaps/, doc/
P="$OUT/4Pane"
mkdir -p "$P"
# shellcheck disable=SC2046
m68k-amigaos-g++ -noixemul -m68040 -specs="$HERE/scripts/m68k-soft-m68040.specs" ${FPUSPECS:+-specs="$FPUSPECS" -L"$FPULIB"} -o "$B/4Pane" $(cd "$B" && sed -n 's/^OBJS =//p' Makefile | sed "s|\([^ ]*\.o\)|$B/\1|g") \
    "$XZ/inst/lib/liblzma.a" $("$W" --libs core,base,xml,html,adv,xrc) 2>&1 | grep -v 'duplicate section' | tail -40
[ -f "$B/4Pane" ] || { echo "no link"; exit 1; }
m68k-amigaos-strip -o "$P/4Pane" "$B/4Pane"
rm -rf "$P/rc" "$P/bitmaps" "$P/doc"
cp -a "$SRC/rc" "$SRC/bitmaps" "$SRC/doc" "$P/"
python3 -I "$HERE/scripts/make-icon.py" "$P/4Pane.info"
ls -l "$P/4Pane"

# the package: 4Pane-<version>.lha holding the 4Pane drawer
rm -rf "$WORK/pkg"; mkdir -p "$WORK/pkg"
cp -a "$P" "$WORK/pkg/"
cp "$HERE/README.md" "$WORK/pkg/4Pane/ReadMe.txt"
mkdir -p "$WORK/pkg/4Pane/Rexx"; cp "$HERE"/Rexx/*.rexx "$WORK/pkg/4Pane/Rexx/"   # the ARexx example (README: ARexx)
mkdir -p "$WORK/pkg/4Pane/tests"; cp "$HERE/tests/arexx.rexx" "$WORK/pkg/4Pane/tests/"   # the ARexx port test
rm -f "$OUT/4Pane-$FP_VERSION.lha"
if command -v lha >/dev/null 2>&1; then
    (cd "$WORK/pkg" && lha -aq "$OUT/4Pane-$FP_VERSION.lha" 4Pane >/dev/null) && ls -l "$OUT/4Pane-$FP_VERSION.lha"
elif command -v jlha >/dev/null 2>&1; then
    (cd "$WORK/pkg" && jlha -aq "$OUT/4Pane-$FP_VERSION.lha" 4Pane >/dev/null) && ls -l "$OUT/4Pane-$FP_VERSION.lha"
else
    echo "no lha here: no package"
fi
