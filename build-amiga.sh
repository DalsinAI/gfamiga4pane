#!/bin/sh
# Builds 4Pane for AmigaOS 3 (68040, the FPU when there is one: fpuauto)
# with a stove: the GFOpenWX Toolkit SDK
# first, then 4Pane (scripts/build-4pane.sh) for what is left of BUDGET_TOTAL
# seconds; the build tree is kept, so the next run goes on.
#   STOVE=.../stoves/os32-gcc16-open6 OUT=out ./build-amiga.sh
#   (WXT_SRC=... to use a toolkit tree already on disk)
# 4Pane is fetched at the version 4pane.lock gives; nothing third-party is
# kept here. The build tree is kept outside the checkout (~/.cache), as a
# Cradle job cleans the checkout first. OUT gets 4Pane/ (the program and its files) when it links.
# Copyright (c) 2026 AmiCradle Team
set -eu
HERE=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
. "$HERE/4pane.lock"
STOVE=${STOVE:-"$HOME/AmiCradle/stoves/os32-gcc16-open6"}
WORK="${WORK:-${XDG_CACHE_HOME:-$HOME/.cache}/gfamiga4pane/build}"; OUT="${OUT:-$WORK/out}"; JOBS=${JOBS:-12}
TC="$STOVE/prefix"
mkdir -p "$WORK/deps" "$OUT"
t0=$(date +%s)

fetch() {   # name repo commit [paths...]: one commit, its tree (or those paths)
    d="$WORK/deps/$1"; repo=$2; c=$3; shift 3
    if [ "$(cat "$d/.commit" 2>/dev/null)" != "$c" ]; then
        rm -rf "${d:?}"; mkdir -p "$d"
        git -C "$d" init -q
        git -C "$d" fetch -q --depth 1 "$repo" "$c"
        if [ $# -gt 0 ]; then git -C "$d" checkout -q FETCH_HEAD -- "$@"; else git -C "$d" checkout -q FETCH_HEAD; fi
        echo "$c" > "$d/.commit"
    fi
}
if [ -z "${WXT_SRC:-}" ]; then fetch gfopenwxtoolkit "$WXT_REPO" "$WXT_COMMIT"; WXT_SRC="$WORK/deps/gfopenwxtoolkit"; fi

# the toolkit again only when its tree changed since its SDK was made
WXB="$WORK/gfopenwx"
WXREV=$( (git -C "$WXT_SRC" rev-parse HEAD 2>/dev/null; git -C "$WXT_SRC" diff HEAD 2>/dev/null) | sha256sum | cut -c1-16)
if [ ! -x "$WXB/out/sdk/bin/wx-config" ] || [ "$(cat "$WXB/out/.rev" 2>/dev/null)" != "$WXREV" ]; then
    STOVE="$STOVE" WORK="$WXB" OUT="$WXB/out" JOBS=$JOBS sh "$WXT_SRC/build-amiga.sh" > "$WORK.wx.log" 2>&1 || { tail -40 "$WORK.wx.log"; exit 1; }
    [ -x "$WXB/out/sdk/bin/wx-config" ] || { tail -40 "$WORK.wx.log"; echo "no toolkit SDK"; exit 1; }
    echo "$WXREV" > "$WXB/out/.rev"
    echo "toolkit: $(( $(date +%s) - t0 )) s"
fi

# fpuauto: libfpuauto.a from its fpuauto.S, as its build-amiga.sh makes it
fetch fpuauto "$FPUAUTO_REPO" "$FPUAUTO_COMMIT" common/fpuauto
FA="$WORK/deps/fpuauto/common/fpuauto"; FAL="$WORK/fpuauto"
if [ "$(cat "$FAL/.commit" 2>/dev/null)" != "$FPUAUTO_COMMIT" ]; then
    rm -rf "$FAL"; mkdir -p "$FAL/obj"
    "$TC/bin/m68k-amigaos-gcc" -m68040 -DFPUAUTO_COMMON -c "$FA/fpuauto.S" -o "$FAL/obj/fa_common.o"
    for n in $(sed 's/^--wrap=//' "$FA/fpuauto.wraps"); do
        "$TC/bin/m68k-amigaos-gcc" -m68040 -DR_$n -c "$FA/fpuauto.S" -o "$FAL/obj/fa_$n.o"
    done
    "$TC/bin/m68k-amigaos-ar" rcs "$FAL/libfpuauto.a" "$FAL"/obj/fa_*.o
    echo "$FPUAUTO_COMMIT" > "$FAL/.commit"
fi

left=$(( ${BUDGET_TOTAL:-840} - ($(date +%s) - t0) ))
[ "$left" -gt 60 ] || { echo "no time left for 4Pane this run"; exit 0; }
TOOLCHAIN="$TC" WORK="$WORK" OUT="$OUT" JOBS=$JOBS WX_SDK="$WXB/out/sdk" BUDGET=$left \
    FPUSPECS="$FA/fpuauto.specs" FPULIB="$FAL" \
    sh "$HERE/scripts/build-4pane.sh"
echo "total: $(( $(date +%s) - t0 )) s"
