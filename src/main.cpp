// NppImapDraft - Notepad++ plugin
// Syncs open files as EML drafts to IMAP server
// (C) pijanydrwal - MIT License

#include "IMapDraftPlugin.h"
#include "ImapClient.h"
#include "EmailMessage.h"
#include "SettingsDialog.h"
#include <tchar.h>
#include <string>
#include <vector>
#include <thread>

// ---- Globals ----
static HINSTANCE g_hInst     = nullptr;
static HWND      g_hNpp      = nullptr;
static HWND      g_hScintilla = nullptr;
static ImapConfig g_config;
static std::string g_iniPath;

// Forward declarations
void cmdSyncDrafts();
void cmdSettings();
void cmdSyncCurrent();

// Plugin menu items
static FuncItem g_funcItems[] = {
    { TEXT("Sync All Open Files to IMAP Drafts"), cmdSyncDrafts,   0, false, nullptr },
    { TEXT("Sync Current File"),                  cmdSyncCurrent,  0, false, nullptr },
    { TEXT("Settings..."),                         cmdSettings,     0, false, nullptr },
};

static const int NB_FUNC = sizeof(g_funcItems) / sizeof(g_funcItems[0]);

// ---- Helper: get INI path ----
static std::string getIniPath() {
    TCHAR buf[MAX_PATH];
    SendMessage(g_hNpp, NPPM_GETPLUGINSCONFIGDIR, MAX_PATH, (LPARAM)buf);
    std::string dir;
    for (int i = 0; buf[i]; i++) dir += (char)buf[i];
    return dir + "\\NppImapDraft.ini";
}

// ---- Helper: get text of current Scintilla buffer ----
static std::string getCurrentBufferText() {
    int len = (int)SendMessage(g_hScintilla, SCI_GETTEXTLENGTH, 0, 0);
    if (len <= 0) return "";
    std::string buf(len + 1, '\0');
    SendMessage(g_hScintilla, SCI_GETTEXT, len + 1, (LPARAM)buf.data());
    buf.resize(len);
    return buf;
}

// ---- Helper: get full path of current file ----
static std::string getCurrentFilePath() {
    TCHAR buf[MAX_PATH];
    SendMessage(g_hNpp, NPPM_GETFULLCURRENTPATH, MAX_PATH, (LPARAM)buf);
    std::string path;
    for (int i = 0; buf[i]; i++) path += (char)buf[i];
    return path;
}

// ---- Helper: extract filename from path ----
static std::string baseName(const std::string& path) {
    auto p = path.rfind('\\');
    if (p == std::string::npos) p = path.rfind('/');
    return (p == std::string::npos) ? path : path.substr(p + 1);
}

// ---- Progress dialog implementation ----
ProgressDlg::ProgressDlg(HWND parent, const std::string& title) {
    INITCOMMONCONTROLSEX icex = { sizeof(icex), ICC_PROGRESS_CLASS };
    InitCommonControlsEx(&icex);

    m_hDlg = CreateWindowExA(0, "#32770", title.c_str(),
                              WS_POPUP | WS_CAPTION | WS_VISIBLE,
                              200, 200, 400, 120,
                              parent, nullptr, g_hInst, nullptr);
    m_hLabel = CreateWindowA("STATIC", "Preparing...",
                              WS_CHILD | WS_VISIBLE, 10, 10, 380, 20,
                              m_hDlg, nullptr, nullptr, nullptr);
    m_hBar = CreateWindowA(PROGRESS_CLASSA, nullptr,
                            WS_CHILD | WS_VISIBLE | PBS_SMOOTH,
                            10, 40, 370, 24,
                            m_hDlg, nullptr, nullptr, nullptr);
    SendMessage(m_hBar, PBM_SETRANGE, 0, MAKELPARAM(0, 100));
    UpdateWindow(m_hDlg);
}

ProgressDlg::~ProgressDlg() { close(); }

void ProgressDlg::setProgress(int cur, int total, const std::string& msg) {
    if (!m_hDlg) return;
    SetWindowTextA(m_hLabel, msg.c_str());
    int pct = (total > 0) ? (cur * 100 / total) : 0;
    SendMessage(m_hBar, PBM_SETPOS, pct, 0);
    MSG wmsg;
    while (PeekMessage(&wmsg, nullptr, 0, 0, PM_REMOVE)) {
        TranslateMessage(&wmsg); DispatchMessage(&wmsg);
    }
}

void ProgressDlg::close() {
    if (m_hDlg) { DestroyWindow(m_hDlg); m_hDlg = nullptr; }
}

// ---- Sync logic ----
static bool syncBuffer(ImapClient& client,
                       const std::string& filename,
                       const std::string& content) {
    EmailMessage msg = EmailMessage::fromBuffer(filename, content);
    std::string eml = msg.toEML();
    return client.appendToDrafts(eml, g_config.draftsFolder);
}

// ---- Plugin commands ----
void cmdSyncCurrent() {
    SettingsDialog sd(g_hInst, g_hNpp);
    sd.setConfig(g_config);
    sd.loadConfig(g_iniPath);
    g_config = sd.getConfig();

    if (g_config.server.empty()) {
        MessageBoxA(g_hNpp, "IMAP server not configured. Open Settings first.",
                    "NppImapDraft", MB_ICONWARNING);
        return;
    }

    std::string path    = getCurrentFilePath();
    std::string content = getCurrentBufferText();
    std::string name    = baseName(path);
    if (name.empty()) name = "Untitled";

    ImapClient client;
    client.connect(g_config);
    if (!client.isConnected()) {
        MessageBoxA(g_hNpp, ("Connect failed: " + client.getLastError()).c_str(),
                    "NppImapDraft", MB_ICONERROR);
        return;
    }
    bool ok = syncBuffer(client, name, content);
    client.disconnect();
    MessageBoxA(g_hNpp,
                ok ? ("Draft saved: " + name).c_str()
                   : ("APPEND failed: " + client.getLastError()).c_str(),
                "NppImapDraft",
                ok ? MB_ICONINFORMATION : MB_ICONERROR);
}

void cmdSyncDrafts() {
    SettingsDialog sd(g_hInst, g_hNpp);
    sd.setConfig(g_config);
    sd.loadConfig(g_iniPath);
    g_config = sd.getConfig();

    if (g_config.server.empty()) {
        MessageBoxA(g_hNpp, "IMAP server not configured. Open Settings first.",
                    "NppImapDraft", MB_ICONWARNING);
        return;
    }

    // Collect open file paths via NPPM_GETOPENFILENAMES
    int nFiles = (int)SendMessage(g_hNpp, NPPM_GETNBOPENFILES, 0, 0);
    if (nFiles <= 0) {
        MessageBoxA(g_hNpp, "No open files.", "NppImapDraft", MB_ICONINFORMATION);
        return;
    }

    std::vector<TCHAR*> paths(nFiles);
    for (auto& p : paths) p = new TCHAR[MAX_PATH]();
    SendMessage(g_hNpp, NPPM_GETOPENFILENAMES, (WPARAM)paths.data(), nFiles);

    ImapClient client;
    client.connect(g_config);
    if (!client.isConnected()) {
        for (auto p : paths) delete[] p;
        MessageBoxA(g_hNpp, ("Connect failed: " + client.getLastError()).c_str(),
                    "NppImapDraft", MB_ICONERROR);
        return;
    }

    ProgressDlg prog(g_hNpp, "Syncing to IMAP Drafts...");
    int ok = 0, fail = 0;

    for (int i = 0; i < nFiles; i++) {
        std::string fp;
        for (int j = 0; paths[i][j]; j++) fp += (char)paths[i][j];
        std::string name = baseName(fp);

        prog.setProgress(i, nFiles, "Uploading: " + name
                         + " (" + std::to_string(i+1) + "/" + std::to_string(nFiles) + ")");

        // Read file from disk (since we can't easily switch Scintilla views)
        std::ifstream ifs(fp, std::ios::binary);
        if (!ifs) { fail++; continue; }
        std::string content((std::istreambuf_iterator<char>(ifs)),
                             std::istreambuf_iterator<char>());

        if (syncBuffer(client, name, content)) ok++;
        else fail++;
    }
    prog.close();
    client.disconnect();
    for (auto p : paths) delete[] p;

    std::string msg = "Done! Uploaded: " + std::to_string(ok) +
                      "  Failed: " + std::to_string(fail);
    MessageBoxA(g_hNpp, msg.c_str(), "NppImapDraft", MB_ICONINFORMATION);
}

void cmdSettings() {
    SettingsDialog sd(g_hInst, g_hNpp);
    sd.setConfig(g_config);
    sd.loadConfig(g_iniPath);
    if (sd.show()) {
        g_config = sd.getConfig();
        sd.saveConfig(g_iniPath);
    }
}

// ================================================================
// Notepad++ plugin DLL entry points (exported)
// ================================================================
extern "C" {

__declspec(dllexport)
void setInfo(HWND nppHandle, HWND scintillaMainHandle,
             HWND scintillaSecondHandle, HINSTANCE hInst, TCHAR* configDir) {
    g_hNpp      = nppHandle;
    g_hScintilla = scintillaMainHandle;
    g_hInst     = hInst;
    // Build INI path from configDir
    std::string cd;
    for (int i = 0; configDir[i]; i++) cd += (char)configDir[i];
    g_iniPath = cd + "\\NppImapDraft.ini";
    // Load saved settings
    SettingsDialog sd(hInst, nppHandle);
    sd.loadConfig(g_iniPath);
    g_config = sd.getConfig();
}

__declspec(dllexport)
const TCHAR* getName() {
    return TEXT("NppImapDraft");
}

__declspec(dllexport)
FuncItem* getFuncsArray(int* nbF) {
    *nbF = NB_FUNC;
    return g_funcItems;
}

__declspec(dllexport)
void beNotified(void* notifyCode) {
    // Not used currently
    (void)notifyCode;
}

__declspec(dllexport)
LRESULT messageProc(UINT msg, WPARAM wParam, LPARAM lParam) {
    return TRUE;
}

__declspec(dllexport)
BOOL isUnicode() { return TRUE; }

} // extern "C"

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID) {
    if (reason == DLL_PROCESS_ATTACH) {
        g_hInst = hModule;
        DisableThreadLibraryCalls(hModule);
    }
    return TRUE;
}
