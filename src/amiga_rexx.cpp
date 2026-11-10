/*
 * amiga_rexx.cpp: 4Pane's ARexx port, 4PANE.1, on the GFOpenWX Toolkit's
 * wxARexxPort and arexx.class (patch 0006 makes it with the main window):
 *   GOTO PATH/A     the active pane shows PATH (a drawer, or a file's drawer
 *                   with the file selected)
 *   GETPATH         RESULT: the active pane's selected path
 *   GETSELECTED     RESULT: every selected path in the active pane, each in
 *                   double quotes, one space between
 *   REFRESH         the active pane reads its drawer again
 * and the toolkit's QUIT, ACTIVATE, DEACTIVATE, VERSION, HELP, FAULT.
 * RC 10 when there is no active pane.
 *
 * Copyright (c) 2026 AmiCradle Team
 */
#include "MyFrame.h"
#include "MyGenericDirCtrl.h"
#include <wx/filefn.h>
#include <wx/openlayout/arexx.h>

static MyGenericDirCtrl *ActivePane(MyFrame *f, wxARexxCommand &c)
{
    MyGenericDirCtrl *p = f->GetActivePane();
    if (p == NULL)
        c.SetError(10);
    return p;
}

void AmigaRexxStart(MyFrame *f)
{
    wxARexxPort *rexx = new wxARexxPort("4PANE", "4Pane " FP_VERSION_STR);
    rexx->Add("GOTO", "PATH/A", [f](wxARexxCommand &c) {
        MyGenericDirCtrl *p = ActivePane(f, c);
        if (p == NULL)
            return;
        wxString path = c.Str(0);
        if (!wxDirExists(path) && !wxFileExists(path))
            c.SetError(10, 205);    /* object not found */
        else
            p->OnOutsideSelect(path);
    });
    rexx->Add("GETPATH", "", [f](wxARexxCommand &c) {
        if (MyGenericDirCtrl *p = ActivePane(f, c))
            c.SetResult(p->GetPath());
    });
    rexx->Add("GETSELECTED", "", [f](wxARexxCommand &c) {
        MyGenericDirCtrl *p = ActivePane(f, c);
        if (p == NULL)
            return;
        wxArrayString paths;
        p->GetMultiplePaths(paths);
        wxString r;
        for (size_t i = 0; i < paths.GetCount(); i++)
            r += (i ? wxT(" \"") : wxT("\"")) + paths[i] + wxT("\"");
        c.SetResult(r);
    });
    rexx->Add("REFRESH", "", [f](wxARexxCommand &c) {
        if (MyGenericDirCtrl *p = ActivePane(f, c))
            p->RefreshTree(p->GetPath());
    });
    rexx->Open();
    f->Bind(wxEVT_DESTROY, [f, rexx](wxWindowDestroyEvent &ev) {
        if (ev.GetEventObject() == f)
            delete rexx;
        ev.Skip();
    });
}
