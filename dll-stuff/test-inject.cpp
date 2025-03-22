#include <Windows.h>
#include <cstdio>

DWORD WINAPI MainThread(LPVOID param) {
    while (true) {
        if (GetAsyncKeyState(VK_F6) & 0x8000) {
            MessageBoxA(NULL, "F6 pressed!", "F6 Pressed!", MB_OK);
        }
        Sleep(100);
    }
    return 0;
}

bool WINAPI DllMain(HINSTANCE hModule, DWORD dwReason, LPVOID lpReserved) {
    if (dwReason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hModule);
        // AllocConsole(); // Create console window
        freopen("CONOUT$", "w", stdout);
        printf("DLL injected! from dll\n");

        MessageBoxA(NULL, "DLL injected!", "DLL injected!", MB_OK);

        HANDLE hThread = CreateThread(0, 0, MainThread, hModule, 0, 0);
        if (hThread) CloseHandle(hThread);
    }
    else if (dwReason == DLL_PROCESS_DETACH) {
        MessageBoxA(NULL, "DLL unloaded!", "Cleanup", MB_OK);
    }
    return true;
}
