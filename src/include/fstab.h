/*
 * fstab.h for 4Pane on the Amiga: there is no /etc/fstab. The calls are
 * there (src/amiga_unix.c) and find nothing, so 4Pane lists no fstab
 * devices; the Amiga's volumes come from DOS instead.
 * Copyright (c) 2026 AmiCradle Team
 */
#ifndef AMIGA4PANE_FSTAB_H
#define AMIGA4PANE_FSTAB_H

#define _PATH_FSTAB "/etc/fstab"
#define FSTAB_RW "rw"
#define FSTAB_RQ "rq"
#define FSTAB_RO "ro"
#define FSTAB_SW "sw"
#define FSTAB_XX "xx"

struct fstab {
    char *fs_spec;
    char *fs_file;
    char *fs_vfstype;
    char *fs_mntops;
    const char *fs_type;
    int fs_freq;
    int fs_passno;
};

#ifdef __cplusplus
extern "C" {
#endif
struct fstab *getfsent(void);
struct fstab *getfsspec(const char *name);
struct fstab *getfsfile(const char *name);
int setfsent(void);
void endfsent(void);
#ifdef __cplusplus
}
#endif

#endif
