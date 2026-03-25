#include "SettingsDialog.h"
#include <commctrl.h>
#include <fstream>
#include <sstream>
#pragma comment(lib, "comctl32.lib")

// We build the dialog entirely in code - no .rc file needed
static SettingsDialog* g_dlgThis = nullptr;

SettingsDialog::SettingsDialog(HINSTANCE hInst, HWND hParent)
    : m_hInst(hInst), m_hParent(hParent), m_hDlg(nullptr) {
    m_config.port = 993;
    m_config.useSSL = true;
    m_config.draftsFolder = "Drafts";
}

SettingsDialog::~SettingsDialog() {}

static HWND addLabel(HWND hDlg, int x, int y, int w, int h, const char* text) {
    return CreateWindowA("STATIC", text, WS_CHILD | WS_VISIBLE,
                         x, y, w, h, hDlg, nullptr, nullptr, nullptr);
}
static HWND addEdit(HWND hDlg, int id, int x, int y, int w, int h,
                    bool password = false) {
    DWORD style = WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL;
    if (password) style |= ES_PASSWORD;
    return CreateWindowA("EDIT", "", style, x, y, w, h, hDlg,
                         (HMENU)(INT_PTR)id, nullptr, nullptr);
}
static HWND addButton(HWND hDlg, int id, int x, int y, int w, int h,
                      const char* text) {
    return CreateWindowA("BUTTON", text,
                         WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                         x, y, w, h, hDlg, (HMENU)(INT_PTR)id,
                         nullptr, nullptr);
}
static HWND addCheck(HWND hDlg, int id, int x, int y, int w, int h,
                     const char* text) {
    return CreateWindowA("BUTTON", text,
                         WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
                         x, y, w, h, hDlg, (HMENU)(INT_PTR)id,
                         nullptr, nullptr);
}

INT_PTR CALLBACK SettingsDialog::dlgProc(HWND hDlg, UINT msg,
                                          WPARAM wParam, LPARAM lParam) {
    if (g_dlgThis) return g_dlgThis->handleMessage(hDlg, msg, wParam, lParam);
    return FALSE;
}

bool SettingsDialog::show() {
    g_dlgThis = this;
    // Create a popup window manually
    WNDCLASSA wc = {};
    wc.lpfnWndProc   = [](HWND h, UINT m, WPARAM w, LPARAM l) -> LRESULT {
        if (g_dlgThis) return g_dlgThis->handleMessage(h, m, w, l);
        return DefWindowProcA(h, m, w, l);
    };
    wc.hInstance     = m_hInst;
    wc.lpszClassName = "NppImapSettingsDlg";
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    wc.hCursor       = LoadCursor(nullptr, IDC_ARROW);
    RegisterClassA(&wc);

    m_hDlg = CreateWindowExA(
        WS_EX_DLGMODALFRAME, "NppImapSettingsDlg",
        "IMAP Draft Settings",
        WS_POPUP | WS_CAPTION | WS_SYSMENU | WS_VISIBLE,
        100, 100, 380, 320,
        m_hParent, nullptr, m_hInst, nullptr);

    if (!m_hDlg) return false;

    // Create controls
    addLabel(m_hDlg,  10,  12, 100, 20, "IMAP Server:");
    addEdit (m_hDlg, IDC_SERVER, 115, 10, 180, 22);
    addLabel(m_hDlg,  10,  42, 100, 20, "Port:");
    addEdit (m_hDlg, IDC_PORT,   115, 40,  60, 22);
    addCheck(m_hDlg, IDC_SSL,    190, 42, 100, 20, "Use SSL/TLS");
    addLabel(m_hDlg,  10,  72, 100, 20, "Username:");
    addEdit (m_hDlg, IDC_USER,   115, 70, 180, 22);
    addLabel(m_hDlg,  10, 102, 100, 20, "Password:");
    addEdit (m_hDlg, IDC_PASS,   115,100, 180, 22, true);
    addLabel(m_hDlg,  10, 132, 100, 20, "Drafts Folder:");
    addEdit (m_hDlg, IDC_FOLDER, 115,130, 180, 22);

    addButton(m_hDlg, IDC_AUTODET, 10, 162, 130, 24, "Auto-detect server");
    addButton(m_hDlg, IDC_TEST,   150, 162, 100, 24, "Test connection");

    CreateWindowA("STATIC", "", WS_CHILD | WS_VISIBLE | SS_SUNKEN,
                  10, 196, 350, 40, m_hDlg, (HMENU)IDC_STATUS, nullptr, nullptr);

    addButton(m_hDlg, IDOK,     200, 248,  70, 26, "OK");
    addButton(m_hDlg, IDCANCEL, 280, 248,  70, 26, "Cancel");

    populateFields(m_hDlg);
    EnableWindow(m_hParent, FALSE);

    MSG msg;
    bool result = false;
    while (GetMessage(&msg, nullptr, 0, 0)) {
        if (msg.message == WM_QUIT) break;
        if (!IsDialogMessage(m_hDlg, &msg)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
    EnableWindow(m_hParent, TRUE);
    SetForegroundWindow(m_hParent);
    g_dlgThis = nullptr;
    return (bool)(INT_PTR)GetWindowLongPtrA(m_hDlg, GWLP_USERDATA);
}

INT_PTR SettingsDialog::handleMessage(HWND hDlg, UINT msg,
                                       WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDOK:
            readFields(hDlg);
            SetWindowLongPtrA(hDlg, GWLP_USERDATA, 1);
            DestroyWindow(hDlg);
            PostQuitMessage(0);
            return TRUE;
        case IDCANCEL:
            SetWindowLongPtrA(hDlg, GWLP_USERDATA, 0);
            DestroyWindow(hDlg);
            PostQuitMessage(0);
            return TRUE;
        case IDC_TEST: {
            readFields(hDlg);
            HWND hStatus = GetDlgItem(hDlg, IDC_STATUS);
            SetWindowTextA(hStatus, "Connecting...");
            ImapClient client;
            if (client.connect(m_config)) {
                SetWindowTextA(hStatus, "OK - Connection successful!");
                client.disconnect();
            } else {
                std::string err = "FAIL: " + client.getLastError();
                SetWindowTextA(hStatus, err.c_str());
            }
            return TRUE;
        }
        case IDC_AUTODET:
            readFields(hDlg);
            autoDetectServer(hDlg);
            return TRUE;
        }
        break;
    case WM_DESTROY:
        return 0;
    }
    return DefWindowProcA(hDlg, msg, wParam, lParam);
}

void SettingsDialog::populateFields(HWND hDlg) {
    SetDlgItemTextA(hDlg, IDC_SERVER, m_config.server.c_str());
    SetDlgItemInt (hDlg, IDC_PORT,   m_config.port, FALSE);
    SetDlgItemTextA(hDlg, IDC_USER,   m_config.username.c_str());
    SetDlgItemTextA(hDlg, IDC_PASS,   m_config.password.c_str());
    SetDlgItemTextA(hDlg, IDC_FOLDER, m_config.draftsFolder.c_str());
    CheckDlgButton(hDlg, IDC_SSL, m_config.useSSL ? BST_CHECKED : BST_UNCHECKED);
}

void SettingsDialog::readFields(HWND hDlg) {
    char buf[512];
    GetDlgItemTextA(hDlg, IDC_SERVER, buf, 512); m_config.server   = buf;
    m_config.port = GetDlgItemInt(hDlg, IDC_PORT, nullptr, FALSE);
    GetDlgItemTextA(hDlg, IDC_USER,   buf, 512); m_config.username = buf;
    GetDlgItemTextA(hDlg, IDC_PASS,   buf, 512); m_config.password = buf;
    GetDlgItemTextA(hDlg, IDC_FOLDER, buf, 512); m_config.draftsFolder = buf;
    m_config.useSSL = (IsDlgButtonChecked(hDlg, IDC_SSL) == BST_CHECKED);
}

void SettingsDialog::autoDetectServer(HWND hDlg) {
    // Try to guess server from username domain
    char buf[256];
    GetDlgItemTextA(hDlg, IDC_USER, buf, 256);
    std::string user(buf);
    auto at = user.find('@');
    if (at == std::string::npos) {
        SetDlgItemTextA(hDlg, IDC_STATUS, "Enter email address first");
        return;
    }
    std::string domain = user.substr(at + 1);
    std::string server = "imap." + domain;
    // Known providers
    if (domain == "gmail.com")    server = "imap.gmail.com";
    else if (domain == "outlook.com" || domain == "hotmail.com" ||
             domain == "live.com")  server = "outlook.office365.com";
    else if (domain == "yahoo.com") server = "imap.mail.yahoo.com";
    else if (domain == "wp.pl")     server = "imap.wp.pl";
    else if (domain == "onet.pl")   server = "imap.poczta.onet.pl";
    else if (domain == "interia.pl") server = "poczta.interia.pl";
    SetDlgItemTextA(hDlg, IDC_SERVER, server.c_str());
    SetDlgItemInt  (hDlg, IDC_PORT,   993, FALSE);
    CheckDlgButton (hDlg, IDC_SSL, BST_CHECKED);
    SetDlgItemTextA(hDlg, IDC_STATUS, ("Detected: " + server).c_str());
}

bool SettingsDialog::saveConfig(const std::string& iniPath) {
    std::ofstream f(iniPath);
    if (!f) return false;
    f << "[imap]\n";
    f << "server=" << m_config.server << "\n";
    f << "port=" << m_config.port << "\n";
    f << "username=" << m_config.username << "\n";
    f << "password=" << m_config.password << "\n";
    f << "folder=" << m_config.draftsFolder << "\n";
    f << "ssl=" << (m_config.useSSL ? "1" : "0") << "\n";
    return true;
}

bool SettingsDialog::loadConfig(const std::string& iniPath) {
    std::ifstream f(iniPath);
    if (!f) return false;
    std::string line;
    while (std::getline(f, line)) {
        if (line.empty() || line[0] == '[' || line[0] == ';') continue;
        auto eq = line.find('=');
        if (eq == std::string::npos) continue;
        std::string key = line.substr(0, eq);
        std::string val = line.substr(eq + 1);
        if (key == "server")   m_config.server = val;
        else if (key == "port") m_config.port = std::stoi(val);
        else if (key == "username") m_config.username = val;
        else if (key == "password") m_config.password = val;
        else if (key == "folder") m_config.draftsFolder = val;
        else if (key == "ssl") m_config.useSSL = (val == "1");
    }
    return true;
}
