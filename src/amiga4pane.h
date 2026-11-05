/*
 * amiga4pane.h: included first in every 4Pane file built for the Amiga
 * (-include, after the toolkit's amigacompat.h). 4Pane tests GTK's version
 * in #if lines that are only meant for wxGTK; there is no GTK here.
 *
 * Copyright (c) 2026 AmiCradle Team
 */
#ifndef AMIGA4PANE_H
#define AMIGA4PANE_H

#ifndef GTK_CHECK_VERSION
#define GTK_CHECK_VERSION(major, minor, micro) 0
#endif

#endif
