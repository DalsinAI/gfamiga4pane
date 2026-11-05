/*
 * amiga_pwent.c: setpwent, endpwent, setgrent and endgrent for the one
 * user and group of amiga_unix.c. Here, so libsocket's own (which bring its
 * getpwuid and getpwnam, already in the toolkit) stay out. Kept apart from
 * pwd.h and grp.h, whose declarations of these differ by C library.
 * Copyright (c) 2026 AmiCradle Team
 */
extern int amiga4pane_pw_next, amiga4pane_gr_next;

void setpwent(void) { amiga4pane_pw_next = 0; }
void endpwent(void) { amiga4pane_pw_next = 0; }
void setgrent(void) { amiga4pane_gr_next = 0; }
void endgrent(void) { amiga4pane_gr_next = 0; }
