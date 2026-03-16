#include <windows.h>
#include <npp/PluginInterface.h>

// DLL entry point
BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
        case DLL_PROCESS_ATTACH:
            break;
        case DLL_THREAD_ATTACH:
            break;
        case DLL_THREAD_DETACH:
            break;
        case DLL_PROCESS_DETACH:
            break;
    }
    return TRUE;
}

extern "C" __declspec(dllexport) void plugin_init(HANDLE hModule) {
    // Initialization code for the Notepad++ plugin
}

extern "C" __declspec(dllexport) void plugin_final() {
    // Cleanup code for the Notepad++ plugin
}