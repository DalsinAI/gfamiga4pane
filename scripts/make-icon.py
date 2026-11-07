#!/usr/bin/env python3
# Writes 4Pane's Workbench tool icon (a classic DiskObject, 4 colours):
# a double-click runs the program with the stack it needs.
#   python3 make-icon.py OUT/4Pane/4Pane.info
# Copyright (c) 2026 AmiCradle Team
import struct
import sys

W, H, DEPTH = 48, 26, 2
STACK = 1048576

# 0 grey, 1 black, 2 white, 3 blue (the Workbench pens)
def picture():
    px = [[0] * W for _ in range(H)]
    for x in range(2, W - 2):          # frame
        px[1][x] = 2
        px[H - 2][x] = 1
    for y in range(1, H - 1):
        px[y][2] = 2
        px[y][W - 3] = 1
    # four panes: two trees (blue) and two file lists (lines)
    for x0 in (4, 24):
        for y in range(3, H - 3):
            px[y][x0 + 9] = 1                  # split between tree and list
        for i, y in enumerate(range(4, H - 4, 3)):
            for x in range(x0 + 1 + (i % 3), x0 + 6):
                px[y][x] = 3                   # tree branches
            for x in range(x0 + 11, x0 + 18):
                px[y][x] = 1                   # file names
    for y in range(2, H - 2):
        px[y][22] = 2                          # the two twin-pane halves
        px[y][23] = 1
    return px

def planes(px):
    row_words = (W + 15) // 16
    out = b""
    for plane in range(DEPTH):
        for y in range(H):
            bits = 0
            for x in range(row_words * 16):
                bit = (px[y][x] >> plane) & 1 if x < W else 0
                bits = (bits << 1) | bit
            out += bits.to_bytes(row_words * 2, "big")
    return out

def main():
    data = planes(picture())
    gadget = struct.pack(">IhhHHHHHIIIIIHI",
                         0, 0, 0, W, H + 1,
                         0x0004,          # GFLG_GADGIMAGE, complement when selected
                         0x0003,          # RELVERIFY | GADGIMMEDIATE
                         0x0001,          # BOOLGADGET
                         1, 0, 0, 0, 0, 0,
                         1)               # UserData 1: a 2.x icon
    disk = struct.pack(">HH", 0xE310, 1) + gadget + struct.pack(">BBIIiiIII",
                         3, 0,            # WBTOOL
                         0, 0,            # no default tool, no tool types
                         -0x80000000, -0x80000000,   # NO_ICON_POSITION
                         0, 0, STACK)
    assert len(disk) == 78, len(disk)
    image = struct.pack(">hhHHHIBBI", 0, 0, W, H, DEPTH, 1, 3, 0, 0)
    with open(sys.argv[1], "wb") as f:
        f.write(disk + image + data)

if __name__ == "__main__":
    main()
