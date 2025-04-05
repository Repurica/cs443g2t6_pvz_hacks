// #include <Windows.h>
// #include <iostream>
// #include <thread>
// #include <ctime>
// #include <cstdlib>

// // Offsets
// #define PATCH_ADDR (BYTE*)(0x0041BAA6)  // Real RVA depends on your base, verify it's 0x1BAA6 + base
// BYTE originalBytes[6];  // Backup original instruction

// LPVOID hiddenSunPtr = nullptr;

// // 🛠 Hook that replaces original instruction
// void WriteHook() {
//     DWORD oldProtect;
//     VirtualProtect(PATCH_ADDR, 6, PAGE_EXECUTE_READWRITE, &oldProtect);

//     // mov [hiddenSunPtr], esi = 89 35 ?? ?? ?? ??   ; opcode + address (6 bytes)
//     BYTE patch[6] = { 0x89, 0x35 }; // mov [addr], esi
//     memcpy(&patch[2], &hiddenSunPtr, sizeof(DWORD));
//     memcpy(originalBytes, PATCH_ADDR, 6);  // backup
//     memcpy(PATCH_ADDR, patch, 6);

//     VirtualProtect(PATCH_ADDR, 6, oldProtect, &oldProtect);
// }

// // 🔁 ASLR-like relocation
// void RelocateMemory() {
//     while (true) {
//         std::this_thread::sleep_for(std::chrono::seconds(3));

//         // Save current sun value
//         int oldValue = 0;
//         memcpy(&oldValue, hiddenSunPtr, sizeof(int));

//         // Free and allocate new
//         VirtualFree(hiddenSunPtr, 0, MEM_RELEASE);
//         hiddenSunPtr = VirtualAlloc(NULL, sizeof(int), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

//         if (hiddenSunPtr) {
//             memcpy(hiddenSunPtr, &oldValue, sizeof(int));
//             WriteHook(); // Reapply hook with new address
//         }
//     }
// }

// DWORD WINAPI MainThread(LPVOID) {
//     srand((unsigned int)time(NULL));
//     hiddenSunPtr = VirtualAlloc(NULL, sizeof(int), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
//     int initVal = 1000;
//     memcpy(hiddenSunPtr, &initVal, sizeof(int));

//     WriteHook();

//     std::thread(RelocateMemory).detach();
//     return 0;
// }

// BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID) {
//     if (reason == DLL_PROCESS_ATTACH) {
//         DisableThreadLibraryCalls(hModule);
//         CreateThread(NULL, 0, MainThread, NULL, 0, NULL);
//     }
//     return TRUE;
// }


// DLL_ASLR_SunProtector.cpp
#include <Windows.h>
#include <iostream>
#include <thread>

// This is the address of the instruction: mov [edi+5560], esi
#define PATCH_ADDR (BYTE*)0x0041BAA6
BYTE originalBytes[6];
LPVOID hiddenSunPtr = nullptr;

// Function to patch original instruction to redirect write to hidden memory
void PatchInstruction() {
    DWORD oldProtect;
    VirtualProtect(PATCH_ADDR, 6, PAGE_EXECUTE_READWRITE, &oldProtect);

    // mov [hiddenSunPtr], esi => opcode: 89 35 [address]
    BYTE patch[6] = { 0x89, 0x35 };  // mov [addr], esi
    DWORD addr = (DWORD)(SIZE_T)hiddenSunPtr;
    memcpy(&patch[2], &addr, sizeof(DWORD));

    // Backup original bytes
    memcpy(originalBytes, PATCH_ADDR, 6);
    // Overwrite with patch
    memcpy(PATCH_ADDR, patch, 6);

    VirtualProtect(PATCH_ADDR, 6, oldProtect, &oldProtect);
}

// Restore original instruction on detach
void RestoreOriginal() {
    DWORD oldProtect;
    VirtualProtect(PATCH_ADDR, 6, PAGE_EXECUTE_READWRITE, &oldProtect);
    memcpy(PATCH_ADDR, originalBytes, 6);
    VirtualProtect(PATCH_ADDR, 6, oldProtect, &oldProtect);
}

DWORD WINAPI MainThread(LPVOID) {
    hiddenSunPtr = VirtualAlloc(nullptr, sizeof(int), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    int initSun = 9999;
    memcpy(hiddenSunPtr, &initSun, sizeof(int));

    PatchInstruction();

    // (Optional) update the hidden value periodically, or hook other logic
    return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID) {
    if (reason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hModule);
        CreateThread(nullptr, 0, MainThread, nullptr, 0, nullptr);
    } else if (reason == DLL_PROCESS_DETACH) {
        RestoreOriginal();
        VirtualFree(hiddenSunPtr, 0, MEM_RELEASE);
    }
    return TRUE;
}