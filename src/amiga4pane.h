/*
 * amiga4pane.h: included first in every 4Pane file built for the Amiga
 * (-include, after the toolkit's amigacompat.h).
 *
 * Copyright (c) 2026 AmiCradle Team
 */
#ifndef AMIGA4PANE_H
#define AMIGA4PANE_H

/* 4Pane tests GTK's version in #if lines only meant for wxGTK */
#ifndef GTK_CHECK_VERSION
#define GTK_CHECK_VERSION(major, minor, micro) 0
#endif

#include <sys/types.h>
#include <pwd.h>
#include <grp.h>

#ifdef __cplusplus
extern "C" {
#endif
/* The reentrant user and group calls libnix lacks (src/amiga_unix.c): the
 * Amiga has one user, so they know that user and its group alone. */
int getpwent_r(struct passwd *pwd, char *buf, size_t buflen, struct passwd **result);
int getpwnam_r(const char *name, struct passwd *pwd, char *buf, size_t buflen, struct passwd **result);
int getpwuid_r(uid_t uid, struct passwd *pwd, char *buf, size_t buflen, struct passwd **result);
int getgrent_r(struct group *grp, char *buf, size_t buflen, struct group **result);
int getgrnam_r(const char *name, struct group *grp, char *buf, size_t buflen, struct group **result);
int getgrgid_r(gid_t gid, struct group *grp, char *buf, size_t buflen, struct group **result);
/* no device files: fails with ENOSYS */
int mknod(const char *path, mode_t mode, dev_t dev);
#ifdef __cplusplus
}
#endif

#endif
