#include <Windows.h>
#include <TlHelp32.h>
#include <iostream>

DWORD GetProcessID(const char* procName) {
    PROCESSENTRY32 pe;
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    pe.dwSize = sizeof(PROCESSENTRY32);

    if (Process32First(snapshot, &pe)) {
        do {
            if (!strcmp(procName, pe.szExeFile)) {
                CloseHandle(snapshot);
                return pe.th32ProcessID;
            }
        } while (Process32Next(snapshot, &pe));
    }
    CloseHandle(snapshot);
    return 0;
}

int main() {
    DWORD pid = GetProcessID("PlantsVsZombies.exe");
    if (!pid) {
        std::cerr << "[❌] Game not found.\n";
        return 1;
    }

    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
    if (!hProcess) {
        std::cerr << "[❌] Could not open process.\n";
        return 1;
    }

    const char* dllPath = "C:\\Users\\Enqi\\Documents\\GitHub\\cs443g2t6_pvz_hacks\\Prevention\\mov_address.dll";
    void* alloc = VirtualAllocEx(hProcess, 0, strlen(dllPath), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    WriteProcessMemory(hProcess, alloc, dllPath, strlen(dllPath), nullptr);

    HANDLE hThread = CreateRemoteThread(hProcess, 0, 0, (LPTHREAD_START_ROUTINE)LoadLibraryA, alloc, 0, 0);
    WaitForSingleObject(hThread, INFINITE);

    std::cout << "[✅] DLL injected!\n";
    CloseHandle(hProcess);
}
