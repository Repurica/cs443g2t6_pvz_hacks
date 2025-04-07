#include <windows.h>
#include <iostream>

bool InjectDLL(DWORD processID, const char* dllPath) {
    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processID);
    if (!hProcess) {
        std::cerr << "Failed to open target process." << std::endl;
        return false;
    }

    LPVOID pDllPath = VirtualAllocEx(hProcess, NULL, strlen(dllPath) + 1, MEM_COMMIT, PAGE_READWRITE);
    if (!pDllPath) {
        std::cerr << "Failed to allocate memory in target process." << std::endl;
        CloseHandle(hProcess);
        return false;
    }

    if (!WriteProcessMemory(hProcess, pDllPath, (LPVOID)dllPath, strlen(dllPath) + 1, NULL)) {
        std::cerr << "Failed to write DLL path to target process memory." << std::endl;
        VirtualFreeEx(hProcess, pDllPath, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return false;
    }

    HMODULE hKernel32 = GetModuleHandle("Kernel32");
    if (!hKernel32) {
        std::cerr << "Failed to get handle to Kernel32 module." << std::endl;
        VirtualFreeEx(hProcess, pDllPath, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return false;
    }

    LPVOID pLoadLibrary = (LPVOID)GetProcAddress(hKernel32, "LoadLibraryA");
    if (!pLoadLibrary) {
        std::cerr << "Failed to get address of LoadLibraryA function." << std::endl;
        VirtualFreeEx(hProcess, pDllPath, 0, MEM_RELEASE);

        CloseHandle(hProcess);
        return false;
    }

    HANDLE hThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)pLoadLibrary, pDllPath, 0, NULL);
    if (!hThread) {
        std::cerr << "Failed to create remote thread in target process." << std::endl;
        VirtualFreeEx(hProcess, pDllPath, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return false;
    }

    WaitForSingleObject(hThread, INFINITE);

    VirtualFreeEx(hProcess, pDllPath, 0, MEM_RELEASE);
    CloseHandle(hThread);
    CloseHandle(hProcess);

    return true;
}

int main() {
    DWORD processID;
    char dllPath[MAX_PATH];


    HWND hGameWindow = FindWindow(NULL, "Plants vs. Zombies");
    DWORD pID = 0;
    GetWindowThreadProcessId(hGameWindow, &pID);



    strcpy(dllPath, "C:\\code\\cs443g2t6_pvz_hacks\\PlantModTemplate\\dllmain.cpp");
    if (InjectDLL(pID, dllPath)) {
        std::cout << "DLL injected successfully." << std::endl;
    } else {
        std::cerr << "DLL injection failed." << std::endl;
    }

    return 0;
}