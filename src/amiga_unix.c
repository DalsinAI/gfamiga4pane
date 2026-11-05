/*
 * amiga_unix.c: the Unix calls 4Pane makes that libnix lacks.
 *
 * - Users and groups: the Amiga has one user. The reentrant calls know that
 *   user ("user", the id getuid() gives) and its group ("users").
 * - fstab and the mount table: no such files. They find nothing; the
 *   Amiga's volumes come from DOS.
 * - Pseudo-terminals: none, and no fork: forkpty fails with ENOSYS.
 *
 * Copyright (c) 2026 AmiCradle Team
 */
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <grp.h>
#include "include/fstab.h"
#include "include/mntent.h"
#include "include/pty.h"

#define ONE_USER "user"
#define ONE_GROUP "users"

/* copies s into buf at *at, or returns NULL when buf is too small */
static char *keep(char *buf, size_t buflen, size_t *at, const char *s)
{
    size_t n = strlen(s) + 1;
    char *p;
    if (*at + n > buflen)
        return NULL;
    p = buf + *at;
    memcpy(p, s, n);
    *at += n;
    return p;
}

static int fill_user(struct passwd *pwd, char *buf, size_t buflen, struct passwd **result)
{
    size_t at = 0;
    memset(pwd, 0, sizeof(*pwd));
    pwd->pw_uid = getuid();
    pwd->pw_gid = getgid();
    if (!(pwd->pw_name = keep(buf, buflen, &at, ONE_USER))
        || !(pwd->pw_passwd = keep(buf, buflen, &at, ""))
        || !(pwd->pw_dir = keep(buf, buflen, &at, "SYS:"))
        || !(pwd->pw_shell = keep(buf, buflen, &at, "C:Shell")))
    {
        *result = NULL;
        return ERANGE;
    }
    *result = pwd;
    return 0;
}

static int fill_group(struct group *grp, char *buf, size_t buflen, struct group **result)
{
    size_t at = 0;
    char **mem;
    memset(grp, 0, sizeof(*grp));
    grp->gr_gid = getgid();
    if (!(grp->gr_name = keep(buf, buflen, &at, ONE_GROUP))
        || !(grp->gr_passwd = keep(buf, buflen, &at, "")))
    {
        *result = NULL;
        return ERANGE;
    }
    /* the member list: the one user, then NULL, aligned in buf */
    at = (at + sizeof(char *) - 1) & ~(sizeof(char *) - 1);
    if (at + 2 * sizeof(char *) > buflen)
    {
        *result = NULL;
        return ERANGE;
    }
    mem = (char **)(buf + at);
    at += 2 * sizeof(char *);
    mem[1] = NULL;
    if (!(mem[0] = keep(buf, buflen, &at, ONE_USER)))
    {
        *result = NULL;
        return ERANGE;
    }
    grp->gr_mem = mem;
    *result = grp;
    return 0;
}

/* the enumerations give the one entry, then NULL, then start again */
static int pw_next, gr_next;

int getpwent_r(struct passwd *pwd, char *buf, size_t buflen, struct passwd **result)
{
    if (pw_next)
    {
        pw_next = 0;
        *result = NULL;
        return ENOENT;
    }
    pw_next = 1;
    return fill_user(pwd, buf, buflen, result);
}

int getpwnam_r(const char *name, struct passwd *pwd, char *buf, size_t buflen, struct passwd **result)
{
    if (!name || strcmp(name, ONE_USER) != 0)
    {
        *result = NULL;
        return 0;
    }
    return fill_user(pwd, buf, buflen, result);
}

int getpwuid_r(uid_t uid, struct passwd *pwd, char *buf, size_t buflen, struct passwd **result)
{
    if (uid != getuid())
    {
        *result = NULL;
        return 0;
    }
    return fill_user(pwd, buf, buflen, result);
}

int getgrent_r(struct group *grp, char *buf, size_t buflen, struct group **result)
{
    if (gr_next)
    {
        gr_next = 0;
        *result = NULL;
        return ENOENT;
    }
    gr_next = 1;
    return fill_group(grp, buf, buflen, result);
}

int getgrnam_r(const char *name, struct group *grp, char *buf, size_t buflen, struct group **result)
{
    if (!name || strcmp(name, ONE_GROUP) != 0)
    {
        *result = NULL;
        return 0;
    }
    return fill_group(grp, buf, buflen, result);
}

int getgrgid_r(gid_t gid, struct group *grp, char *buf, size_t buflen, struct group **result)
{
    if (gid != getgid())
    {
        *result = NULL;
        return 0;
    }
    return fill_group(grp, buf, buflen, result);
}

/* ---- fstab and the mount table: none ---- */

int setfsent(void) { return 0; }
void endfsent(void) {}
struct fstab *getfsent(void) { return NULL; }
struct fstab *getfsspec(const char *name) { (void)name; return NULL; }
struct fstab *getfsfile(const char *name) { (void)name; return NULL; }

FILE *setmntent(const char *file, const char *mode)
{
    (void)file; (void)mode;
    errno = ENOENT;
    return NULL;
}
struct mntent *getmntent(FILE *stream) { (void)stream; return NULL; }
int addmntent(FILE *stream, const struct mntent *mnt) { (void)stream; (void)mnt; return 1; }
int endmntent(FILE *stream) { (void)stream; return 1; }
char *hasmntopt(const struct mntent *mnt, const char *opt) { (void)mnt; (void)opt; return NULL; }

/* ---- pseudo-terminals: none ---- */

pid_t forkpty(int *amaster, char *name, const struct termios *termp, const struct winsize *winp)
{
    (void)amaster; (void)name; (void)termp; (void)winp;
    errno = ENOSYS;
    return -1;
}

int openpty(int *amaster, int *aslave, char *name, const struct termios *termp, const struct winsize *winp)
{
    (void)amaster; (void)aslave; (void)name; (void)termp; (void)winp;
    errno = ENOSYS;
    return -1;
}
