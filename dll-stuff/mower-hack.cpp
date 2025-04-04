#include <Windows.h>
#include <iostream>
#include <TlHelp32.h>
#include <string>
#include <sstream>
#include <vector>

bool isActivated = false;
HANDLE sunHackThread = NULL;

// Function to get the process ID of the game
DWORD GetProcessID(const wchar_t* processName) {
    DWORD processID = 0;
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot != INVALID_HANDLE_VALUE) {
        PROCESSENTRY32W pe; // Use the wide-character version
        pe.dwSize = sizeof(pe);
        if (Process32FirstW(hSnapshot, &pe)) {
            do {
                if (_wcsicmp(pe.szExeFile, processName) == 0) {
                    processID = pe.th32ProcessID;
                    break;
                }
            } while (Process32NextW(hSnapshot, &pe));
        }
        CloseHandle(hSnapshot);
    }
    return processID;
}



VOID mower_activate(HANDLE processHandle){
    DWORD_PTR baseAddress=0x6A9EC0;
    DWORD_PTR offset;
    int buffer=2;
    

    ReadProcessMemory(processHandle, (LPVOID)baseAddress, &baseAddress, 4, NULL);
    offset=0x768;

    ReadProcessMemory(processHandle, (LPVOID)(baseAddress+offset), &baseAddress, 4, NULL);
    offset=0x100;
    
    ReadProcessMemory(processHandle, (LPVOID)(baseAddress+offset), &baseAddress, 4, NULL);

    WriteProcessMemory(processHandle, (LPVOID)(baseAddress+0x2c), &buffer, sizeof(buffer), nullptr);
    WriteProcessMemory(processHandle, (LPVOID)(baseAddress+0x2c+0x48), &buffer, sizeof(buffer), nullptr);
    WriteProcessMemory(processHandle, (LPVOID)(baseAddress+0x2c+0x48+0x48), &buffer, sizeof(buffer), nullptr);
    WriteProcessMemory(processHandle, (LPVOID)(baseAddress+0x2c+0x48+0x48+0x48), &buffer, sizeof(buffer), nullptr);
    WriteProcessMemory(processHandle, (LPVOID)(baseAddress+0x2c+0x48+0x48+0x48+0x48), &buffer, sizeof(buffer), nullptr);

}

VOID mower_reset(HANDLE processHandle) {
    DWORD_PTR baseAddress = 0x6A9EC0;
    DWORD_PTR offset;
    int buffer = 2;

    ReadProcessMemory(processHandle, (LPVOID)baseAddress, &baseAddress, 4, NULL);
    offset = 0x768;

    ReadProcessMemory(processHandle, (LPVOID)(baseAddress + offset), &baseAddress, 4, NULL);
    offset = 0x100;

    ReadProcessMemory(processHandle, (LPVOID)(baseAddress + offset), &baseAddress, 4, NULL);

    float position = -20.0f; // Use float type for the position
    WriteProcessMemory(processHandle, (LPVOID)(0x679D58), &position, sizeof(position), nullptr); // Use sizeof(buffer) to ensure correct byte size

    for (int i = 0; i < 5; i++) {
        DWORD_PTR currentBaseAddress;
        ReadProcessMemory(processHandle, (LPVOID)(baseAddress + (i * 0x48)), &currentBaseAddress, sizeof(currentBaseAddress), nullptr);
        
        BYTE shellcode[] = {
            0x60,                               // pushad
            0x68, (BYTE)(currentBaseAddress & 0xFF), (BYTE)((currentBaseAddress >> 8) & 0xFF), (BYTE)((currentBaseAddress >> 16) & 0xFF), (BYTE)((currentBaseAddress >> 24) & 0xFF), // push currentBaseAddress
            0xB8, (BYTE)(i & 0xFF), (BYTE)((i >> 8) & 0xFF), (BYTE)((i >> 16) & 0xFF), (BYTE)((i >> 24) & 0xFF), // mov eax, i
            0xE8, 0x60, 0x81, 0x45, 0x00,       // call 00458160
            0x61,                               // popad
            0xC3                                // ret
        };
        // Update shellcode to display a MessageBoxA
        BYTE messageBoxShellcode[] = {
            0x60,                               // pushad
            0x68, 0x00, 0x00, 0x00, 0x00,       // push 0 (MB_OK)
            0x68, 0x00, 0x00, 0x00, 0x00,       // push address of "Title"
            0x68, 0x00, 0x00, 0x00, 0x00,       // push address of "Message"
            0xB8, 0x00, 0x00, 0x00, 0x00,       // mov eax, address of MessageBoxA
            0xFF, 0xD0,                         // call eax
            0x61,                               // popad
            0xC3                                // ret
        };

        // Allocate memory for strings and MessageBoxA address
        LPVOID remoteMessage = VirtualAllocEx(processHandle, nullptr, 256, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
        if (remoteMessage) {
            const char* message = "Hello from shellcode!";
            const char* title = "Shellcode MessageBox";
            FARPROC messageBoxAddr = GetProcAddress(GetModuleHandleA("user32.dll"), "MessageBoxA");

            // Write strings and function address to remote process
            WriteProcessMemory(processHandle, remoteMessage, message, strlen(message) + 1, nullptr);
            WriteProcessMemory(processHandle, (LPVOID)((BYTE*)remoteMessage + 64), title, strlen(title) + 1, nullptr);
            *(DWORD*)&messageBoxShellcode[6] = (DWORD)((BYTE*)remoteMessage + 64); // Title
            *(DWORD*)&messageBoxShellcode[11] = (DWORD)remoteMessage;             // Message
            *(DWORD*)&messageBoxShellcode[16] = (DWORD)messageBoxAddr;            // MessageBoxA address

            // Allocate memory for shellcode
            LPVOID remoteShellcode = VirtualAllocEx(processHandle, nullptr, sizeof(messageBoxShellcode), MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
            if (remoteShellcode) {
            WriteProcessMemory(processHandle, remoteShellcode, messageBoxShellcode, sizeof(messageBoxShellcode), nullptr);
            HANDLE thread = CreateRemoteThread(processHandle, nullptr, 0, (LPTHREAD_START_ROUTINE)remoteShellcode, nullptr, 0, nullptr);
            if (thread) {
                WaitForSingleObject(thread, INFINITE);
                CloseHandle(thread);
            }
            VirtualFreeEx(processHandle, remoteShellcode, 0, MEM_RELEASE);
            }
            VirtualFreeEx(processHandle, remoteMessage, 0, MEM_RELEASE);
        }
        LPVOID remoteMemory = VirtualAllocEx(processHandle, nullptr, sizeof(shellcode), MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
        if (remoteMemory) {
            WriteProcessMemory(processHandle, remoteMemory, shellcode, sizeof(shellcode), nullptr);
            HANDLE thread = CreateRemoteThread(processHandle, nullptr, 0, (LPTHREAD_START_ROUTINE)remoteMemory, nullptr, 0, nullptr);
            if (thread) {
                WaitForSingleObject(thread, INFINITE);
                CloseHandle(thread);
            }
            VirtualFreeEx(processHandle, remoteMemory, 0, MEM_RELEASE);
        }
    }
}

// Main thread to listen for F6 key press and start the sun hack
DWORD WINAPI MainThread(LPVOID param) {
    HANDLE hProcess = (HANDLE)param;  // Retrieve the process handle passed as a parameter

    while (true) {
        if (GetAsyncKeyState(VK_F5) & 0x8000) {
            mower_activate(hProcess);  // Call activate on F5 press
            Sleep(500);  // Sleep to prevent multiple activations within a short time
        }
        if (GetAsyncKeyState(VK_F6) & 0x8000) {
            mower_reset(hProcess);  // Call reset on F6 press
            Sleep(500);  // Sleep to prevent multiple activations within a short time
        }
        Sleep(100);  // Sleep to reduce CPU usage
    }

    CloseHandle(hProcess);
    return 0;
}

// DLL entry point
bool WINAPI DllMain(HINSTANCE hModule, DWORD dwReason, LPVOID lpReserved) {
    if (dwReason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hModule);
        // AllocConsole(); // Create console window for debugging
        MessageBoxA(NULL, "DLL injected!", "DLL injected!", MB_OK);
        DWORD processID = GetProcessID(L"PlantsVsZombies.exe");
        if (processID) {
            HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processID);
            if (hProcess) {
            CreateThread(NULL, 0, MainThread, hProcess, 0, NULL); // Start main loop to listen for keypresses
            } else {
            MessageBoxA(NULL, "Failed to open process!", "Error", MB_OK);
            }
        } else {
            MessageBoxA(NULL, "Failed to find the process!", "Error", MB_OK);
        }
    }
    return true;
}
