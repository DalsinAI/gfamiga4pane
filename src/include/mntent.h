/*
 * mntent.h for 4Pane on the Amiga: there is no mount table file. The calls
 * are there (src/amiga_unix.c); setmntent gives NULL until the Amiga's
 * volumes are listed through it.
 * Copyright (c) 2026 AmiCradle Team
 */
#ifndef AMIGA4PANE_MNTENT_H
#define AMIGA4PANE_MNTENT_H

#include <stdio.h>

#define _PATH_MOUNTED "/etc/mtab"
#define _PATH_MNTTAB "/etc/fstab"
#define MOUNTED _PATH_MOUNTED
#define MNTTAB _PATH_MNTTAB

struct mntent {
    char *mnt_fsname;
    char *mnt_dir;
    char *mnt_type;
    char *mnt_opts;
    int mnt_freq;
    int mnt_passno;
};

#ifdef __cplusplus
extern "C" {
#endif
FILE *setmntent(const char *file, const char *mode);
struct mntent *getmntent(FILE *stream);
int addmntent(FILE *stream, const struct mntent *mnt);
int endmntent(FILE *stream);
char *hasmntopt(const struct mntent *mnt, const char *opt);
#ifdef __cplusplus
}
#endif

#endif
