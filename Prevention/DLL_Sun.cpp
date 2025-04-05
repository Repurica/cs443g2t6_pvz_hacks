
#include <windows.h>
#include <iostream>
#include <psapi.h>
#include <vector>

// Globals
DWORD_PTR g_sunAddress = 0;
int g_backupSun = 0;
HANDLE g_hProcess = nullptr;

LONG WINAPI VectoredHandler(PEXCEPTION_POINTERS pExceptionInfo) {
    if (pExceptionInfo->ExceptionRecord->ExceptionCode == STATUS_GUARD_PAGE_VIOLATION) {
        PVOID faultAddr = reinterpret_cast<PVOID>(pExceptionInfo->ExceptionRecord->ExceptionInformation[1]);


        if ((DWORD_PTR)faultAddr == g_sunAddress) {
            // Attempt to modify sun value detected!
            std::cerr << "[!] Sun memory access detected! Checking caller...\n";

            // Get module handle of caller
            HMODULE hMods[1024];
            DWORD cbNeeded;
            if (EnumProcessModules(g_hProcess, hMods, sizeof(hMods), &cbNeeded)) {
                for (unsigned int i = 0; i < (cbNeeded / sizeof(HMODULE)); ++i) {
                    MODULEINFO mi;
                    GetModuleInformation(g_hProcess, hMods[i], &mi, sizeof(mi));
                    if ((DWORD_PTR)faultAddr >= (DWORD_PTR)mi.lpBaseOfDll &&
                        (DWORD_PTR)faultAddr <= ((DWORD_PTR)mi.lpBaseOfDll + mi.SizeOfImage)) {
                        char modName[MAX_PATH];
                        GetModuleBaseNameA(g_hProcess, hMods[i], modName, sizeof(modName));
                        std::cout << "[INFO] Write came from module: " << modName << std::endl;

                        if (strcmp(modName, "PlantsVsZombies.exe") != 0) {
                            std::cerr << "[!] Tampering detected. Reverting sun value.\n";
                            WriteProcessMemory(g_hProcess, (LPVOID)g_sunAddress, &g_backupSun, sizeof(int), NULL);
                        }
                    }
                }
            }

            // Reapply PAGE_GUARD
            DWORD oldProtect;
            VirtualProtect((LPVOID)g_sunAddress, sizeof(int), PAGE_READWRITE | PAGE_GUARD, &oldProtect);
            return EXCEPTION_CONTINUE_EXECUTION;
        }
    }

    return EXCEPTION_CONTINUE_SEARCH;
}

int main() {
    HWND hGameWindow = FindWindow(NULL, "Plants vs. Zombies");
    if (!hGameWindow) {
        std::cerr << "[ERROR] Game window not found.\n";
        return 1;
    }

    DWORD pID;
    GetWindowThreadProcessId(hGameWindow, &pID);
    g_hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pID);
    if (!g_hProcess) {
        std::cerr << "[ERROR] Could not open process.\n";
        return 1;
    }

    // Resolve sun memory address (same as before)
    DWORD_PTR baseAddress = 0x6A9EC0;
    DWORD_PTR offset = 0x768;
    ReadProcessMemory(g_hProcess, (LPVOID)baseAddress, &baseAddress, 4, NULL);
    ReadProcessMemory(g_hProcess, (LPVOID)(baseAddress + offset), &baseAddress, 4, NULL);
    g_sunAddress = baseAddress + 0x5560;

    // Backup original value
    ReadProcessMemory(g_hProcess, (LPCVOID)g_sunAddress, &g_backupSun, sizeof(int), NULL);
    std::cout << "[INFO] Original sun value: " << g_backupSun << std::endl;

    // Set PAGE_GUARD
    DWORD oldProtect;
    if (!VirtualProtect((LPVOID)g_sunAddress, sizeof(int), PAGE_READWRITE | PAGE_GUARD, &oldProtect)) {
        std::cerr << "[ERROR] Could not set PAGE_GUARD protection.\n";
        return 1;
    }

    // Install Vectored Exception Handler
    AddVectoredExceptionHandler(1, VectoredHandler);

    std::cout << "[INFO] Monitoring sun memory for tampering...\n";
    while (true) {
        Sleep(1000);
    }
    return 0;
}
