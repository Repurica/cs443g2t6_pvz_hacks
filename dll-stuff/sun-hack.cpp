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

void* FindPattern(HANDLE hProcess, const char* pattern, size_t patternLength) {
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);

    DWORD_PTR startAddress = (DWORD_PTR)sysInfo.lpMinimumApplicationAddress;
    DWORD_PTR endAddress = (DWORD_PTR)sysInfo.lpMaximumApplicationAddress;

    std::vector<BYTE> buffer(4096);
    SIZE_T bytesRead;

    // Scan through the memory range
    for (DWORD_PTR address = startAddress; address < endAddress; address += buffer.size()) {
        if (ReadProcessMemory(hProcess, (LPCVOID)address, buffer.data(), buffer.size(), &bytesRead)) {
            for (size_t i = 0; i < bytesRead - patternLength; i++) {
                if (memcmp(&buffer[i], pattern, patternLength) == 0) {
                    return (void*)(address + i);
                }
            }
        }
    }
    return nullptr; // Pattern not found
}

// Function to find the address of the sun value in the game process
DWORD FindSunAddress(HANDLE hProcess, int sunValueBefore) {
    DWORD_PTR baseAddress = 0x6A9EC0;
    DWORD_PTR sunAddress;
    DWORD_PTR plant_num_address;

    DWORD_PTR offset=0x768;
    int plant_num;
    ReadProcessMemory(hProcess, (LPVOID)baseAddress, &baseAddress, 4, NULL);
    ReadProcessMemory(hProcess, (LPVOID)(baseAddress+offset), &baseAddress, 4, NULL);
    sunAddress=baseAddress+0x5560;

    return sunAddress; // Return the sun address found


    // Define the signature pattern (example)
    // const char pattern[] = { 0x12, 0x34, 0x56, 0x78 }; // Replace with actual signature in the game's memory
    // size_t patternLength = sizeof(pattern);

    // // Search for the pattern in memory
    // void* patternAddress = FindPattern(hProcess, pattern, patternLength);
    // if (patternAddress) {
    //     DWORD_PTR sunValueAddress = (DWORD_PTR)patternAddress;

    //     // Use the found address and calculate the correct offset dynamically
    //     DWORD_PTR offset = 0x1000; // Replace with the actual offset from the pattern
    //     DWORD_PTR sunAddress = sunValueAddress + offset;

    //     return sunAddress;
    // }
    // return 0;
}

// Function to set the sun value in the game process
void SetSunValue(HANDLE hProcess, DWORD sunAddress, int newValue) {
    BOOL result = WriteProcessMemory(hProcess, (LPVOID)sunAddress, &newValue, sizeof(newValue), NULL);
}

// Function to handle the sun hack
DWORD WINAPI SunHackThread(LPVOID param) {
    DWORD processID = GetProcessID(L"PlantsVsZombies.exe");
    if (!processID) {
        MessageBoxA(NULL, "Failed to find the process!", "Error", MB_OK);
        return 0;
    }

    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processID);
    if (!hProcess) {
        MessageBoxA(NULL, "Failed to open process!", "Error", MB_OK);
        return 0;
    }

    // Find the address of the sun value
    int sunValue = 50; // Initial sun value to search for
    DWORD sunAddress = FindSunAddress(hProcess, sunValue);
    if (sunAddress) {
    //     MessageBoxA(NULL, "Found sun address! Address: " + sunAddress, "Success", MB_OK);

        // Make the sun value unlimited
        while (isActivated) {
            SetSunValue(hProcess, sunAddress, 9999);  // Set sun value to 9999 (unlimited)
            Sleep(100);  // Sleep to reduce CPU usage
        }
    } else {
        MessageBoxA(NULL, "Failed to find sun address!", "Error", MB_OK);
    }

    CloseHandle(hProcess);
    return 0;
}

void ToggleSunHackThread() {
    if (isActivated) {
        // MessageBoxA(NULL, "Deactivating SunHack...", "SunHack Deactivated!", MB_OK);
        isActivated = false;
        if (sunHackThread != NULL) {
            // Signal the SunHack thread to stop and then wait for it to finish
            TerminateThread(sunHackThread, 0);
            CloseHandle(sunHackThread);
            sunHackThread = NULL;
        }
    } else {
        isActivated = true;
        // MessageBoxA(NULL, "Activating SunHack...", "SunHack Activated!", MB_OK);
        sunHackThread = CreateThread(NULL, 0, SunHackThread, NULL, 0, NULL); // Start a new thread
    }
}

// Main thread to listen for F6 key press and start the sun hack
DWORD WINAPI MainThread(LPVOID param) {
    while (true) {
        if (GetAsyncKeyState(VK_F6) & 0x8000) {
            if (GetAsyncKeyState(VK_F6) & 0x8000) {
                ToggleSunHackThread();  // Toggle activation/deactivation on F6 press
                Sleep(500);  // Sleep to prevent multiple activations within a short time
            }
        }
        Sleep(100);  // Sleep to reduce CPU usage
    }
    return 0;
}

// DLL entry point
bool WINAPI DllMain(HINSTANCE hModule, DWORD dwReason, LPVOID lpReserved) {
    if (dwReason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hModule);
        // AllocConsole(); // Create console window for debugging
        MessageBoxA(NULL, "DLL injected!", "DLL injected!", MB_OK);
        CreateThread(NULL, 0, MainThread, hModule, 0, NULL); // Start main loop to listen for keypresses
    }
    return true;
}
