#!/bin/sh
# Builds 4Pane for AmigaOS 3 (68040) with a stove: the GFOpenWX Toolkit SDK
# first, then 4Pane (scripts/build-4pane.sh) for what is left of BUDGET_TOTAL
# seconds; the build tree is kept, so the next run goes on.
#   STOVE=.../stoves/os32-gcc16-open6 OUT=out ./build-amiga.sh
#   (WXT_SRC=... to use a toolkit tree already on disk)
# 4Pane is fetched at the version 4pane.lock gives; nothing third-party is
# kept here. OUT gets 4Pane/ (the program and its files) when it links.
# Copyright (c) 2026 AmiCradle Team
set -eu
HERE=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
. "$HERE/4pane.lock"
STOVE=${STOVE:-"$HOME/AmiCradle/stoves/os32-gcc16-open6"}
OUT="${OUT:-$HERE/build/out}"; WORK="${WORK:-$HERE/build}"; JOBS=${JOBS:-12}
TC="$STOVE/prefix"
mkdir -p "$WORK/deps" "$OUT"
t0=$(date +%s)

fetch() {
    d="$WORK/deps/$1"
    if [ "$(git -C "$d" rev-parse HEAD 2>/dev/null)" != "$3" ]; then
        rm -rf "${d:?}"; mkdir -p "$d"
        git -C "$d" init -q
        git -C "$d" fetch -q --depth 1 "$2" "$3"
        git -C "$d" checkout -q FETCH_HEAD
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

left=$(( ${BUDGET_TOTAL:-840} - ($(date +%s) - t0) ))
[ "$left" -gt 60 ] || { echo "no time left for 4Pane this run"; exit 0; }
TOOLCHAIN="$TC" WORK="$WORK" OUT="$OUT" JOBS=$JOBS WX_SDK="$WXB/out/sdk" BUDGET=$left \
    sh "$HERE/scripts/build-4pane.sh"
echo "total: $(( $(date +%s) - t0 )) s"
