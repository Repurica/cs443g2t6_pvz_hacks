#include <windows.h>
#include <tlhelp32.h>
#include <stdio.h>

DWORD GetProcessIdByName(const char* processName) {
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) return 0;

    PROCESSENTRY32 pe;
    pe.dwSize = sizeof(PROCESSENTRY32);

    if (Process32First(hSnapshot, &pe)) {
        do {
            if (_stricmp(pe.szExeFile, processName) == 0) {
                CloseHandle(hSnapshot);
                return pe.th32ProcessID;
            }
        } while (Process32Next(hSnapshot, &pe));
    }

    CloseHandle(hSnapshot);
    return 0;
}

int main(int argc, char** argv) {
    if (argc < 2) {
        printf("Usage: %s <DLL Path> \n", argv[0]);
        return 1;
    }

    const char* dll_path = argv[1];
    const char* process_name = "PlantsVsZombies.exe"; // Default process name
    DWORD PID = 0;

    printf("Searching for process: %s\n", process_name);
    PID = GetProcessIdByName(process_name);

    if (PID == 0) {
        printf("Failed to find target process.\n");
        return 1;
    }

    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, PID);
    if (hProcess == NULL) {
        printf("Failed to open target process: %d\n", GetLastError());
        return 1;
    }

    LPVOID allocated_mem = VirtualAllocEx(hProcess, NULL, strlen(dll_path) + 1, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (allocated_mem == NULL) {
        printf("Failed to allocate memory in target process: %d\n", GetLastError());
        CloseHandle(hProcess);
        return 1;
    }

    if (!WriteProcessMemory(hProcess, allocated_mem, dll_path, strlen(dll_path) + 1, NULL)) {
        printf("Failed to write memory in target process: %d\n", GetLastError());
        VirtualFreeEx(hProcess, allocated_mem, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return 1;
    }

    HMODULE kernel32Base = GetModuleHandleA("kernel32.dll");
    if (kernel32Base == NULL) {
        printf("Failed to retrieve handle to kernel32.dll: %d\n", GetLastError());
        VirtualFreeEx(hProcess, allocated_mem, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return 1;
    }

    FARPROC load_library_address = GetProcAddress(kernel32Base, "LoadLibraryA");
    if (load_library_address == NULL) {
        printf("Failed to get address of LoadLibraryA: %d\n", GetLastError());
        VirtualFreeEx(hProcess, allocated_mem, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return 1;
    }

    HANDLE hThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)load_library_address, allocated_mem, 0, NULL);
    if (hThread == NULL) {
        printf("Failed to create remote thread: %d\n", GetLastError());
        VirtualFreeEx(hProcess, allocated_mem, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return 1;
    }

    printf("DLL injected successfully.\n");
    WaitForSingleObject(hThread, INFINITE);

    VirtualFreeEx(hProcess, allocated_mem, 0, MEM_RELEASE);
    CloseHandle(hThread);
    CloseHandle(hProcess);

    return 0;
}
