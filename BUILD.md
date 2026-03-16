# Building NppImapDraft Plugin

This document provides a step-by-step guide to compiling the `NppImapDraft.dll` plugin for Notepad++. Follow the sections below to set up your environment, compile the plugin, and install it in Notepad++.

## Prerequisites

Before you start, make sure you have the following tools installed on your system:

1. **Git**:
   - Download and install from [git-scm.com](https://git-scm.com/).

2. **Visual Studio**:
   - Install the latest version of [Visual Studio](https://visualstudio.microsoft.com/).
   - During installation, make sure to include the **Desktop development with C++** workload.

3. **CMake**:
   - Download and install from [cmake.org](https://cmake.org/download/).

4. **OpenSSL**:
   - You will need OpenSSL to build the plugin. Follow the instructions below to install it.

### Installing OpenSSL

1. Download the installer for OpenSSL from [slproweb.com](https://slproweb.com/products/Win32OpenSSL.html).
2. Run the installer and follow the prompts to complete the installation.
3. After installation, add the OpenSSL bin directory to your system's PATH environment variable, which is typically located at `C:\Program Files\OpenSSL-Win64\bin` (for 64-bit systems).

### Visual Studio Setup

1. Open Visual Studio.
2. Clone this repository:
   ```bash
   git clone https://github.com/pijanydrwal/NppImapDraft.git
   ```
3. Open the `NppImapDraft` solution file (`NppImapDraft.sln`) in Visual Studio.

### Building the Project

To compile the `NppImapDraft.dll`, follow these steps:

1. In Visual Studio, select the desired build configuration (Debug/Release) from the toolbar.
2. Right-click on the solution in the Solution Explorer and select **Build**.
3. Once the build completes, the output DLL can be found in the `Debug` or `Release` folder inside the project directory, depending on the selected configuration.

### Installation of the DLL

To install the compiled DLL into Notepad++, follow these steps:

1. Locate the compiled `NppImapDraft.dll` file.
2. Copy the DLL file.
3. Navigate to your Notepad++ installation directory, typically located at:
   ```
   C:\Program Files\Notepad++\plugins\
   ```
4. Create a new folder named `NppImapDraft` if it does not exist.
5. Paste the `NppImapDraft.dll` file into the `NppImapDraft` folder.

### Conclusion

You have successfully built and installed the `NppImapDraft.dll` plugin for Notepad++. Launch Notepad++ to see the plugin in action!