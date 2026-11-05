/*
 * ftw.h for 4Pane on the Amiga: nftw (src/amiga_unix.c), walking a
 * directory tree with opendir and lstat, paths joined the AmigaDOS way.
 * Copyright (c) 2026 AmiCradle Team
 */
#ifndef AMIGA4PANE_FTW_H
#define AMIGA4PANE_FTW_H

#include <sys/stat.h>

enum { FTW_F, FTW_D, FTW_DNR, FTW_NS, FTW_SL, FTW_DP, FTW_SLN };
enum { FTW_PHYS = 1, FTW_MOUNT = 2, FTW_CHDIR = 4, FTW_DEPTH = 8 };

struct FTW {
    int base;
    int level;
};

#ifdef __cplusplus
extern "C" {
#endif
int nftw(const char *path, int (*fn)(const char *, const struct stat *, int, struct FTW *), int fdlimit, int flags);
#ifdef __cplusplus
}
#endif

#endif
