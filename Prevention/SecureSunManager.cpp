// #include <iostream>
// #include <cstdlib>
// #include <ctime>
// #include <thread>
// #include <chrono>
// #include <mutex>
// #include <windows.h>

// class SecureSunManager {
// private:
//     HANDLE hProcess;
//     DWORD_PTR sun_address;
//     int xor_key;
//     int obfuscated_backup_sun;
//     std::mutex lock;

//     // XOR obfuscation function
//     int obfuscate(int value) { return value ^ xor_key; }
//     int deobfuscate(int value) { return value ^ xor_key; }

//     // Memory integrity check
//     void integrity_check() {
//         std::lock_guard<std::mutex> guard(lock);

//         int current_value = 0;
//         ReadProcessMemory(hProcess, (LPCVOID)sun_address, &current_value, sizeof(int), NULL);
//         int expected_value = deobfuscate(obfuscated_backup_sun);

//         if (current_value != expected_value) {
//             std::cerr << "[WARNING] Sun value tampered! Resetting to safe value.\n";
//             WriteProcessMemory(hProcess, (LPVOID)sun_address, &expected_value, sizeof(int), NULL);
//         }
//     }

// public:
//     SecureSunManager(HANDLE processHandle, DWORD_PTR addr) {
//         hProcess = processHandle;
//         sun_address = addr;

//         srand(static_cast<unsigned int>(time(nullptr)));
//         xor_key = rand() % 1000 + 1;

//         int sun_value = 0;
//         ReadProcessMemory(hProcess, (LPCVOID)sun_address, &sun_value, sizeof(int), NULL);
//         obfuscated_backup_sun = obfuscate(sun_value);

//         // Background checker thread
//         std::thread([this]() {
//             while (true) {
//                 std::this_thread::sleep_for(std::chrono::seconds(2));
//                 integrity_check();
//             }
//         }).detach();
//     }
// };

// int main() {
//     HWND hGameWindow = FindWindow(NULL, "Plants vs. Zombies");
//     if (!hGameWindow) {
//         std::cerr << "[ERROR] Game window not found.\n";
//         return 1;
//     }

//     DWORD pID = 0;
//     GetWindowThreadProcessId(hGameWindow, &pID);
//     HANDLE hProc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pID);

//     if (!hProc) {
//         std::cerr << "[ERROR] Could not open process.\n";
//         return 1;
//     }

//     DWORD_PTR baseAddress = 0x6A9EC0;
//     DWORD_PTR offset = 0x768;
    
//     // First dereference
//     ReadProcessMemory(hProc, (LPVOID)baseAddress, &baseAddress, 4, NULL);
    
//     // Second dereference
//     ReadProcessMemory(hProc, (LPVOID)(baseAddress + offset), &baseAddress, 4, NULL);
    
//     // Now add the final offset to get sun_address
//     DWORD_PTR sun_address = baseAddress + 0x5560;

//     SecureSunManager sunProtector(hProc, sun_address);

//     // Keep program running to allow background monitoring
//     while (true) {
//         std::this_thread::sleep_for(std::chrono::seconds(10));
//     }

//     return 0;
// }





#include <iostream>
#include <windows.h>
#include <thread>
#include <chrono>
#include <mutex>
#include <vector>
#include <cmath>

class SecureSunManager {
private:
    HANDLE hProcess;
    DWORD_PTR sun_address;
    int previous_sun;
    std::mutex lock;

    bool is_legit_change(int delta) {
        std::vector<int> legit_increase = {0, 25, 50, 75};
    
        if (delta < 0 && std::abs(delta) <= 200) {
            return true;  // Allow any deduction up to 200
        }
    
        for (int inc : legit_increase) {
            if (delta == inc) return true;
        }
    
        return false;
    }

    void integrity_check() {
        std::lock_guard<std::mutex> guard(lock);
        int current_value = 0;

        if (!ReadProcessMemory(hProcess, (LPCVOID)sun_address, &current_value, sizeof(int), NULL)) {
            std::cerr << "[ERROR] Failed to read sun value.\n";
            return;
        }

        int delta = current_value - previous_sun;

        if (!is_legit_change(delta)) {
            std::cerr << "[ALERT] Abnormal sun change detected! change = " << delta << "\n";
            
            // Revert to previous legitimate sun value
            WriteProcessMemory(hProcess, (LPVOID)sun_address, &previous_sun, sizeof(int), NULL);
            std::cout << "[INFO] Sun value reverted to: " << previous_sun << "\n";
        } else {
            previous_sun = current_value;  // Accept and update
        }
        
    }

public:
    SecureSunManager(HANDLE proc, DWORD_PTR addr) {
        hProcess = proc;
        sun_address = addr;

        ReadProcessMemory(hProcess, (LPCVOID)sun_address, &previous_sun, sizeof(int), NULL);
        std::cout << "[INFO] Initial sun value: " << previous_sun << std::endl;

        std::thread([this]() {
            while (true) {
                std::this_thread::sleep_for(std::chrono::milliseconds(1500));
                integrity_check();
            }
        }).detach();
    }
};

// --- Main to Attach ---

int main() {
    HWND hGameWindow = FindWindow(NULL, "Plants vs. Zombies");
    if (!hGameWindow) {
        std::cerr << "[ERROR] Game window not found.\n";
        return 1;
    }

    DWORD pid;
    GetWindowThreadProcessId(hGameWindow, &pid);
    HANDLE hProc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);

    if (!hProc) {
        std::cerr << "[ERROR] Could not open process.\n";
        return 1;
    }

    DWORD_PTR base = 0x6A9EC0;
    ReadProcessMemory(hProc, (LPCVOID)base, &base, sizeof(base), NULL);
    ReadProcessMemory(hProc, (LPCVOID)(base + 0x768), &base, sizeof(base), NULL);
    DWORD_PTR sun_address = base + 0x5560;

    SecureSunManager protector(hProc, sun_address);

    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(5));
    }

    return 0;
}
