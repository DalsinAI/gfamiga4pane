// amiga_svg.cpp: 4Pane's SvgToPng draws SVG previews through librsvg,
// opened with dlopen (sdk/rsvg/rsvg.cpp). The Amiga has neither, so there
// is no SVG preview: it says so and 4Pane shows none.
// Copyright (c) 2026 AmiCradle Team
#include "wx/string.h"

bool SvgToPng(const wxString& WXUNUSED(svgfilepath), const wxString& WXUNUSED(outputfilepath), void* WXUNUSED(dlhandle))
{
    return false;
}
