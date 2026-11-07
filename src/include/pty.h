/*
 * pty.h for 4Pane on the Amiga: no pseudo-terminals and no fork, so
 * forkpty fails (ENOSYS) and 4Pane's commands take their other way.
 * Copyright (c) 2026 AmiCradle Team
 */
#ifndef AMIGA4PANE_PTY_H
#define AMIGA4PANE_PTY_H

#include <sys/types.h>

struct termios;
struct winsize;

#ifdef __cplusplus
extern "C" {
#endif
pid_t forkpty(int *amaster, char *name, const struct termios *termp, const struct winsize *winp);
int openpty(int *amaster, int *aslave, char *name, const struct termios *termp, const struct winsize *winp);
#ifdef __cplusplus
}
#endif

#endif
