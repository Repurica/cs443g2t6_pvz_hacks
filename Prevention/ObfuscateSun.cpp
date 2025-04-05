#include <iostream>
#include <windows.h>
#include <thread>
#include <chrono>
#include <mutex>
#include <cstdlib>
#include <ctime>

class SunProtector {
private:
    HANDLE hProcess;
    DWORD_PTR sunAddress;
    int xorKey;
    int* obfuscatedBackupPtr = nullptr;
    std::mutex lock;

    int obfuscate(int val) { return val ^ xorKey; }
    int deobfuscate(int val) { return val ^ xorKey; }

    void relocateBackup(int current) {
        std::lock_guard<std::mutex> guard(lock);
        delete obfuscatedBackupPtr;
        xorKey = rand() % 500 + 1;
        obfuscatedBackupPtr = new int(obfuscate(current));
    }

    void checkLoop() {
        while (true) {
            std::this_thread::sleep_for(std::chrono::milliseconds(200));

            int currentValue = 0;
            ReadProcessMemory(hProcess, (LPCVOID)sunAddress, &currentValue, sizeof(int), NULL);

            int backupValue = deobfuscate(*obfuscatedBackupPtr);

            int delta = currentValue - backupValue;

            if (abs(delta) > 200 && !(delta == 25 || delta == 50 || delta == 75)) {
                std::cerr << "[ALERT] Abnormal sun change! Resetting: delta = " << delta << "\n";
                WriteProcessMemory(hProcess, (LPVOID)sunAddress, &backupValue, sizeof(int), NULL);
            } else {
                relocateBackup(currentValue);  // Accept as valid and re-randomise
            }
        }
    }

public:
    SunProtector(HANDLE proc, DWORD_PTR addr) {
        srand(static_cast<unsigned int>(time(nullptr)));
        hProcess = proc;
        sunAddress = addr;

        int initVal = 0;
        ReadProcessMemory(hProcess, (LPCVOID)sunAddress, &initVal, sizeof(int), NULL);
        xorKey = rand() % 500 + 1;
        obfuscatedBackupPtr = new int(obfuscate(initVal));

        std::thread(&SunProtector::checkLoop, this).detach();
    }

    ~SunProtector() {
        delete obfuscatedBackupPtr;
    }
};

int main() {
    HWND hWnd = FindWindow(NULL, "Plants vs. Zombies");
    DWORD pid;
    GetWindowThreadProcessId(hWnd, &pid);
    HANDLE hProc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);

    // Resolve sun address:
    DWORD_PTR base = 0x6A9EC0;
    DWORD_PTR offset = 0x768;
    ReadProcessMemory(hProc, (LPCVOID)base, &base, 4, NULL);
    ReadProcessMemory(hProc, (LPCVOID)(base + offset), &base, 4, NULL);
    DWORD_PTR sunAddr = base + 0x5560;

    SunProtector protector(hProc, sunAddr);

    // Keep alive
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(10));
    }
}
