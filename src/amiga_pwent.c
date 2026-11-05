/*
 * amiga_pwent.c: setpwent and endpwent for the one user of amiga_unix.c
 * (setgrent and endgrent are libsocket's, whose getgrent.o wx brings in
 * anyway; getgrent_r starts again by itself). Here, so libsocket's own (which bring its
 * getpwuid and getpwnam, already in the toolkit) stay out. Kept apart from
 * pwd.h and grp.h, whose declarations of these differ by C library.
 * Copyright (c) 2026 AmiCradle Team
 */
extern int amiga4pane_pw_next;

void setpwent(void) { amiga4pane_pw_next = 0; }
void endpwent(void) { amiga4pane_pw_next = 0; }
