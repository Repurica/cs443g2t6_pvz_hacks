#include <Windows.h>
#include <iostream>
#include <thread>
#include <chrono>

// Backup of original instruction bytes at sun access location
BYTE originalBytes[6];
LPVOID randomizedSunMemory = nullptr;
int actualSunValue = 9999; // Default fallback

// PvZ sun value pointer chain constants
DWORD_PTR baseAddr = 0x6A9EC0;
DWORD_PTR offset1 = 0x768;
DWORD_PTR offset2 = 0x5560;

DWORD patchAddress = 0x004315EF; // Location where game accesses sun value

void PatchGameToUseNewAddress() {
    BYTE patch[6] = { 0xA1 }; // MOV EAX, [address]
    memcpy(&patch[1], &randomizedSunMemory, sizeof(DWORD));

    // Save original bytes first time
    ReadProcessMemory(GetCurrentProcess(), (LPCVOID)patchAddress, originalBytes, sizeof(originalBytes), NULL);
    WriteProcessMemory(GetCurrentProcess(), (LPVOID)patchAddress, patch, sizeof(patch), NULL);
}

void RestorePatch() {
    if (originalBytes[0] != 0) {
        WriteProcessMemory(GetCurrentProcess(), (LPVOID)patchAddress, originalBytes, sizeof(originalBytes), NULL);
    }
}

DWORD_PTR ResolveSunAddress() {
    DWORD_PTR base = baseAddr;
    ReadProcessMemory(GetCurrentProcess(), (LPCVOID)base, &base, sizeof(DWORD), NULL);
    ReadProcessMemory(GetCurrentProcess(), (LPCVOID)(base + offset1), &base, sizeof(DWORD), NULL);
    return base + offset2;
}

void InitSunASLR() {
    DWORD_PTR sunAddress = ResolveSunAddress();

    // Read current sun value
    ReadProcessMemory(GetCurrentProcess(), (LPCVOID)sunAddress, &actualSunValue, sizeof(int), NULL);

    // Allocate randomized memory and write sun
    randomizedSunMemory = VirtualAlloc(NULL, sizeof(int), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    WriteProcessMemory(GetCurrentProcess(), randomizedSunMemory, &actualSunValue, sizeof(int), NULL);

    // Patch the game to read from randomized memory
    PatchGameToUseNewAddress();
}

DWORD WINAPI MainThread(LPVOID) {
    InitSunASLR();

    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(5));

        // Backup current value
        ReadProcessMemory(GetCurrentProcess(), randomizedSunMemory, &actualSunValue, sizeof(int), NULL);

        // Reallocate to new random location
        VirtualFree(randomizedSunMemory, 0, MEM_RELEASE);
        randomizedSunMemory = VirtualAlloc(NULL, sizeof(int), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
        WriteProcessMemory(GetCurrentProcess(), randomizedSunMemory, &actualSunValue, sizeof(int), NULL);

        // Reapply patch with new address
        PatchGameToUseNewAddress();
    }
    return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID) {
    if (reason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hModule);
        CreateThread(NULL, 0, MainThread, NULL, 0, NULL);
    } else if (reason == DLL_PROCESS_DETACH) {
        RestorePatch();
        VirtualFree(randomizedSunMemory, 0, MEM_RELEASE);
    }
    return TRUE;
}
