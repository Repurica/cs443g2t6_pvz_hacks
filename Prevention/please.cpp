#include <windows.h>
#include <iostream>
#include <thread>
#include <mutex>
#include <ctime>
#include <cstdlib>

class XORObfuscatedSun {
private:
    HANDLE hProcess;
    DWORD_PTR sunAddress;
    int xorKey;
    std::mutex lock;

    int obfuscate(int value) {
        return value ^ xorKey;
    }

    int deobfuscate(int value) {
        return value ^ xorKey;
    }

    void writeEncryptedSun(int realValue) {
        int encrypted = obfuscate(realValue);
        WriteProcessMemory(hProcess, (LPVOID)sunAddress, &encrypted, sizeof(int), NULL);
    }

    int readDecryptedSun() {
        int encrypted;
        ReadProcessMemory(hProcess, (LPCVOID)sunAddress, &encrypted, sizeof(int), NULL);
        return deobfuscate(encrypted);
    }

    void monitorSunValue() {
        int lastValid = readDecryptedSun();
        while (true) {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));

            std::lock_guard<std::mutex> guard(lock);
            int currentDecrypted = readDecryptedSun();

            int delta = currentDecrypted - lastValid;

            if (abs(delta) > 200 && delta != 25 && delta != 50 && delta != 75) {
                std::cerr << "[ALERT] Abnormal sun change detected! Reverting...\n";
                writeEncryptedSun(lastValid);
            } else {
                lastValid = currentDecrypted;  // Acceptable update
            }
        }
    }

public:
    XORObfuscatedSun(HANDLE proc, DWORD_PTR addr) : hProcess(proc), sunAddress(addr) {
        srand((unsigned int)time(NULL));
        xorKey = rand() % 1000 + 1;

        int original;
        ReadProcessMemory(hProcess, (LPCVOID)sunAddress, &original, sizeof(int), NULL);
        writeEncryptedSun(original);

        std::thread(&XORObfuscatedSun::monitorSunValue, this).detach();
    }
};

int main() {
    HWND hGameWindow = FindWindow(NULL, "Plants vs. Zombies");
    if (!hGameWindow) {
        std::cerr << "[ERROR] Game not found.\n";
        return 1;
    }

    DWORD pid;
    GetWindowThreadProcessId(hGameWindow, &pid);
    HANDLE hProc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
    if (!hProc) {
        std::cerr << "[ERROR] Failed to open process.\n";
        return 1;
    }

    // Resolve sun address
    DWORD_PTR base = 0x6A9EC0;
    DWORD_PTR offset = 0x768;
    ReadProcessMemory(hProc, (LPCVOID)base, &base, 4, NULL);
    ReadProcessMemory(hProc, (LPCVOID)(base + offset), &base, 4, NULL);
    DWORD_PTR sunAddr = base + 0x5560;

    XORObfuscatedSun protector(hProc, sunAddr);

    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(10));
    }
}
