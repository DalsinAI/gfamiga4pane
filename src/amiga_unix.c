/*
 * amiga_unix.c: the Unix calls 4Pane makes that libnix lacks.
 *
 * - Users and groups: the Amiga has one user. The reentrant calls know that
 *   user ("user", the id getuid() gives) and its group ("users"), and so
 *   do getgrgid and getgrent; the toolkit's tar reader calls getgrgid for
 *   each archive it lists: libsocket's getgrgid never comes back on the Amiga, which
 *   hung 4Pane on opening a .tar.gz or .tar.bz2.
 * - fstab and the mount table: no such files. They find nothing; the
 *   Amiga's volumes come from DOS.
 * - Pseudo-terminals: none, and no fork: forkpty fails with ENOSYS.
 * - nftw: a walk with opendir and lstat; mknod: no device files (ENOSYS).
 * - lchown: no owners (ENOSYS); lutimes: the file's date through utime
 *   (AmigaDOS has no access time and links aren't followed by it anyway).
 * - mkdtemp, here so libnix's mktemp.o (whose mkstemp the toolkit also
 *   has) isn't linked; pause, for 4Pane's debugger wait.
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
#include "include/ftw.h"
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <utime.h>
#include <time.h>

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

/* the enumerations give the one entry, then NULL, then start again;
 * set/end in amiga_pwent.c reset them */
int amiga4pane_pw_next, amiga4pane_gr_next;
#define pw_next amiga4pane_pw_next
#define gr_next amiga4pane_gr_next

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

/* the plain calls, for the toolkit's tar reader (getgrgid) and libsocket's
 * getgroups (getgrent); with setgrent and endgrent (amiga_pwent.c) they keep
 * libsocket's getgrent.o out of the link */
static struct group one_grp;
static char one_grp_buf[64];

struct group *getgrgid(gid_t gid)
{
    struct group *result;
    if (getgrgid_r(gid, &one_grp, one_grp_buf, sizeof(one_grp_buf), &result) != 0)
        return NULL;
    return result;
}

struct group *getgrent(void)
{
    struct group *result;
    if (getgrent_r(&one_grp, one_grp_buf, sizeof(one_grp_buf), &result) != 0)
        return NULL;
    return result;
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

/* ---- device files: none ---- */

int mknod(const char *path, mode_t mode, dev_t dev)
{
    (void)path; (void)mode; (void)dev;
    errno = ENOSYS;
    return -1;
}

/* ---- nftw ---- */

typedef int (*nftw_fn)(const char *, const struct stat *, int, struct FTW *);

/* where the last name in path starts */
static int name_base(const char *path)
{
    int i, base = 0;
    for (i = 0; path[i]; i++)
        if ((path[i] == '/' || path[i] == ':') && path[i + 1])
            base = i + 1;
    return base;
}

static int walk(const char *path, nftw_fn fn, int flags, int level)
{
    struct stat st;
    struct FTW ftw;
    int type, r;
    DIR *d;
    struct dirent *e;

    ftw.base = name_base(path);
    ftw.level = level;
    if (((flags & FTW_PHYS) ? lstat(path, &st) : stat(path, &st)) != 0)
        return fn(path, NULL, FTW_NS, &ftw);
    if (S_ISLNK(st.st_mode))
        return fn(path, &st, FTW_SL, &ftw);
    if (!S_ISDIR(st.st_mode))
        return fn(path, &st, FTW_F, &ftw);

    d = opendir(path);
    type = d ? ((flags & FTW_DEPTH) ? FTW_DP : FTW_D) : FTW_DNR;
    if (!(flags & FTW_DEPTH) || !d)
    {
        r = fn(path, &st, type, &ftw);
        if (r || !d)
        {
            if (d)
                closedir(d);
            return r;
        }
    }
    r = 0;
    while (!r && (e = readdir(d)) != NULL)
    {
        size_t n = strlen(path);
        char *child;
        if (!strcmp(e->d_name, ".") || !strcmp(e->d_name, ".."))
            continue;
        child = malloc(n + strlen(e->d_name) + 2);
        if (!child)
        {
            r = -1;
            break;
        }
        /* "DH0:" and "Work/" take the name as it is; others a slash first */
        memcpy(child, path, n);
        if (n && path[n - 1] != ':' && path[n - 1] != '/')
            child[n++] = '/';
        strcpy(child + n, e->d_name);
        r = walk(child, fn, flags, level + 1);
        free(child);
    }
    closedir(d);
    if (!r && (flags & FTW_DEPTH))
        r = fn(path, &st, FTW_DP, &ftw);
    return r;
}

int nftw(const char *path, nftw_fn fn, int fdlimit, int flags)
{
    (void)fdlimit;
    return walk(path, fn, flags, 0);
}

/* ---- owners, times, temporary directories ---- */

int lchown(const char *path, uid_t owner, gid_t group)
{
    (void)path; (void)owner; (void)group;
    errno = ENOSYS;
    return -1;
}

int lutimes(const char *path, const struct timeval tv[2])
{
    struct utimbuf ub;
    if (!tv)
        return utime(path, NULL);
    ub.actime = tv[0].tv_sec;
    ub.modtime = tv[1].tv_sec;
    return utime(path, &ub);
}

char *mkdtemp(char *tmpl)
{
    static unsigned long seq;
    static const char digits[] = "0123456789abcdefghijklmnopqrstuvwxyz";
    size_t n = strlen(tmpl);
    int tries, i;
    if (n < 6 || strcmp(tmpl + n - 6, "XXXXXX") != 0)
    {
        errno = EINVAL;
        return NULL;
    }
    for (tries = 0; tries < 100; tries++)
    {
        unsigned long v = (unsigned long)time(NULL) * 31UL + ++seq * 7919UL;
        for (i = 0; i < 6; i++)
        {
            tmpl[n - 6 + i] = digits[v % 36];
            v /= 36;
        }
        if (mkdir(tmpl, 0700) == 0)
            return tmpl;
        if (errno != EEXIST)
            return NULL;
    }
    errno = EEXIST;
    return NULL;
}

int pause(void)
{
    errno = ENOSYS;
    return -1;
}
