#ifndef SETTINGS_DIALOG_H
#define SETTINGS_DIALOG_H

#include <windows.h>
#include <string>
#include "ImapClient.h"

class SettingsDialog {
public:
    SettingsDialog(HINSTANCE hInst, HWND hParent);
    ~SettingsDialog();

    // Show modal dialog, returns true if OK was clicked
    bool show();

    // Get/set current config
    ImapConfig getConfig() const { return m_config; }
    void setConfig(const ImapConfig& cfg) { m_config = cfg; }

    // Save/load from INI file next to plugin DLL
    bool saveConfig(const std::string& iniPath);
    bool loadConfig(const std::string& iniPath);

private:
    HINSTANCE m_hInst;
    HWND m_hParent;
    HWND m_hDlg;
    ImapConfig m_config;

    static INT_PTR CALLBACK dlgProc(HWND hDlg, UINT msg,
                                     WPARAM wParam, LPARAM lParam);
    INT_PTR handleMessage(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
    void populateFields(HWND hDlg);
    void readFields(HWND hDlg);
    void autoDetectServer(HWND hDlg);

    // Control IDs
    enum {
        IDC_SERVER  = 101,
        IDC_PORT    = 102,
        IDC_USER    = 103,
        IDC_PASS    = 104,
        IDC_FOLDER  = 105,
        IDC_SSL     = 106,
        IDC_TEST    = 107,
        IDC_STATUS  = 108,
        IDC_AUTODET = 109
    };
};

#endif // SETTINGS_DIALOG_H
