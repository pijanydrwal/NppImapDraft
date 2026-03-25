# NppImapDraft - Build Instructions

## What is this?

A Notepad++ plugin that syncs open files as EML messages to an IMAP **Drafts** folder.
Useful for keeping code/text notes accessible via email client.

## Requirements

- Visual Studio 2019 or 2022 (Community is fine)
- Windows SDK 10.x
- OpenSSL for Windows (optional, needed for SSL/IMAP port 993)
  - Download: https://slproweb.com/products/Win32OpenSSL.html
  - Or compile without SSL using `IMAP_NO_SSL` define (port 143 only)

## Build steps

### 1. Clone / open project

```
git clone https://github.com/pijanydrwal/NppImapDraft.git
cd NppImapDraft
```

Open `NppImapDraft.vcxproj` in Visual Studio.

### 2. Configure OpenSSL paths (if using SSL)

In Visual Studio project properties:
- **C/C++ > Additional Include Directories**: add `C:\OpenSSL-Win64\include`
- **Linker > Additional Library Directories**: add `C:\OpenSSL-Win64\lib`

Or define `IMAP_NO_SSL` in **C/C++ > Preprocessor Definitions** to skip SSL entirely
(plugin will only work with unencrypted IMAP on port 143).

### 3. Build

- Configuration: **Release | x64** (for 64-bit Notepad++) or **Release | Win32** (for 32-bit)
- Build Solution (Ctrl+Shift+B)
- Output: `x64\Release\NppImapDraft.dll`

### 4. Install

Copy `NppImapDraft.dll` to:
```
%APPDATA%\Notepad++\plugins\NppImapDraft\NppImapDraft.dll
```
or:
```
C:\Program Files\Notepad++\plugins\NppImapDraft\NppImapDraft.dll
```

If you used OpenSSL, also copy `libssl-3-x64.dll` and `libcrypto-3-x64.dll`
from OpenSSL bin folder to the same plugin directory.

### 5. Restart Notepad++

The plugin appears under **Plugins > NppImapDraft** with 3 menu items:
- **Sync All Open Files to IMAP Drafts** - uploads all open tabs
- **Sync Current File** - uploads just the active tab
- **Settings...** - configure IMAP server, credentials, folder

## Settings

In the Settings dialog:
- **IMAP Server** - e.g. `imap.gmail.com`
- **Port** - 993 (SSL) or 143 (plain)
- **Username** - your full email address
- **Password** - your email password or app password
- **Drafts Folder** - usually `Drafts` (Gmail uses `[Gmail]/Drafts`)
- **Use SSL/TLS** - check for port 993
- **Auto-detect server** - fills server from email domain
- **Test connection** - verifies settings before saving

## Gmail notes

- Enable IMAP in Gmail settings
- Use an App Password (not your regular password)
- Drafts folder name: `[Gmail]/Szkice` (Polish) or `[Gmail]/Drafts` (English)

## Compilation without Visual Studio (MSBuild CLI)

```bat
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
msbuild NppImapDraft.vcxproj /p:Configuration=Release /p:Platform=x64
```
