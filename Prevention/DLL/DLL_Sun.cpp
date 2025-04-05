#include <Windows.h>
#include <iostream>

BYTE originalBytes[5]; // Only patch 5 bytes for 32-bit MOV
DWORD sunPointerBase = 0x6A9EC0;
DWORD offset1 = 0x768;
DWORD offset2 = 0x5560;

LPVOID hiddenSunMemory = nullptr;
int fakeSunValue = 9999;

void HookSunAccess() {
    DWORD base, temp;
    ReadProcessMemory(GetCurrentProcess(), (LPCVOID)sunPointerBase, &base, sizeof(DWORD), NULL);
    ReadProcessMemory(GetCurrentProcess(), (LPCVOID)(base + offset1), &temp, sizeof(DWORD), NULL);
    DWORD sunAddress = temp + offset2;

    hiddenSunMemory = VirtualAlloc(nullptr, sizeof(int), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    memcpy(hiddenSunMemory, &fakeSunValue, sizeof(int));  // Store fake sun value

    // Patch instruction to read from hidden memory
    DWORD patchAddr = 0x004315EF;
    ReadProcessMemory(GetCurrentProcess(), (LPCVOID)patchAddr, originalBytes, sizeof(originalBytes), NULL);

    BYTE patch[5] = { 0xA1 };  // MOV EAX, [imm32]
    DWORD addr = (DWORD)hiddenSunMemory;
    memcpy(&patch[1], &addr, sizeof(DWORD));  // insert address after opcode

    WriteProcessMemory(GetCurrentProcess(), (LPVOID)patchAddr, patch, sizeof(patch), NULL);
}

void RestoreOriginalBytes() {
    DWORD patchAddr = 0x004315EF;
    WriteProcessMemory(GetCurrentProcess(), (LPVOID)patchAddr, originalBytes, sizeof(originalBytes), NULL);
    if (hiddenSunMemory)
        VirtualFree(hiddenSunMemory, 0, MEM_RELEASE);
}

DWORD WINAPI MainThread(LPVOID param) {
    HookSunAccess();
    return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID reserved) {
    if (reason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hModule);
        CreateThread(NULL, 0, MainThread, NULL, 0, NULL);
    } else if (reason == DLL_PROCESS_DETACH) {
        RestoreOriginalBytes();
    }
    return TRUE;
}
