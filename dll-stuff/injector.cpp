#include <windows.h>
#include <stdio.h>

int main(int argc, char** argv) {
    // Ensure the DLL path is correct
    PCSTR dll_path = "./cheat.dll";

    // Find the game window
    HWND hGameWindow = FindWindowA(NULL, "Plants vs. Zombies");
    if (!hGameWindow) {
        printf("Failed to find game window. Error: %d\n", GetLastError());
        return 1;
    }

    // Get process ID
    DWORD PID = 0;
    GetWindowThreadProcessId(hGameWindow, &PID);
    if (PID == 0) {
        printf("Failed to get process ID. Error: %d\n", GetLastError());
        return 1;
    }

    // Open process with necessary access
    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, PID);
    if (hProcess == NULL) {
        printf("Failed to retrieve handle to remote process: %d\n", GetLastError());
        return 1;
    }

    // Allocate memory in the target process
    LPVOID allocated_mem = VirtualAllocEx(hProcess, NULL, strlen(dll_path) + 1, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (allocated_mem == NULL) {
        printf("Failed to allocate memory in remote process: %d\n", GetLastError());
        CloseHandle(hProcess);
        return 1;
    }
    printf("Memory allocated at: %p\n", allocated_mem);

    // Write DLL path into allocated memory
    if (!WriteProcessMemory(hProcess, allocated_mem, dll_path, strlen(dll_path) + 1, NULL)) {
        printf("Failed to write memory in remote process: %d\n", GetLastError());
        VirtualFreeEx(hProcess, allocated_mem, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return 1;
    }

    // Get the address of LoadLibraryA in kernel32.dll
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

    // Create remote thread to load the DLL
    HANDLE hThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)load_library_address, allocated_mem, 0, NULL);
    if (hThread == NULL) {
        printf("Failed to create remote thread in target process: %d\n", GetLastError());
        VirtualFreeEx(hProcess, allocated_mem, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return 1;
    }

    printf("Thread handle: %p\n", hThread);
    WaitForSingleObject(hThread, INFINITE);

    // Cleanup
    VirtualFreeEx(hProcess, allocated_mem, 0, MEM_RELEASE);
    CloseHandle(hThread);
    CloseHandle(hProcess);

    return 0;
}
