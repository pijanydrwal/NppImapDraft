#ifndef IMAP_DRAFT_PLUGIN_H
#define IMAP_DRAFT_PLUGIN_H

// Notepad++ plugin interface headers
// These define the Notepad++ plugin API
#include <windows.h>
#include <string>
#include <vector>

// ---- Notepad++ plugin SDK types (minimal subset) ----
typedef LRESULT (*PFUNCPLUGINCMD)();

struct ShortcutKey {
    bool _isAlt;
    bool _isCtrl;
    bool _isShift;
    UCHAR _key;
};

struct FuncItem {
    TCHAR _itemName[64];
    PFUNCPLUGINCMD _pFunc;
    int _cmdID;
    bool _init2Check;
    ShortcutKey* _pShKey;
};

// Notepad++ messages
#define NPPM_GETCURRENTDOCINDEX     (WM_USER + 1027)
#define NPPM_GETFULLCURRENTPATH     (WM_USER + 1001)
#define NPPM_GETNBOPENFILES         (WM_USER + 1031)
#define NPPM_GETOPENFILENAMES       (WM_USER + 1032)
#define NPPM_GETPLUGINSHOMEPATH     (WM_USER + 1015)
#define NPPM_GETPLUGINSCONFIGDIR    (WM_USER + 1016)
#define NPPM_SETSTATUSBAR           (WM_USER + 1047)
#define STATUSBAR_DOC_TYPE          0
#define STATUSBAR_DOC_SIZE          1
#define STATUSBAR_CUR_POS           2
#define STATUSBAR_EOF_FORMAT        3
#define STATUSBAR_UNICODE_TYPE      4
#define STATUSBAR_TYPING_MODE       5

// Scintilla messages to get text
#define SCI_GETTEXT                 2182
#define SCI_GETTEXTLENGTH           2183

// Progress bar dialog (we show a simple MessageBox or modeless window)
class ProgressDlg {
public:
    ProgressDlg(HWND parent, const std::string& title);
    ~ProgressDlg();
    void setProgress(int cur, int total, const std::string& msg);
    void close();
private:
    HWND m_hDlg;
    HWND m_hBar;
    HWND m_hLabel;
};

#endif // IMAP_DRAFT_PLUGIN_H
