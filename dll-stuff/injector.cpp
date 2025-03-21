#define UNICODE
#define _UNICODE
#include <Windows.h>
#include <TlHelp32.h>
#include <cstdio>

int getProcId(const wchar_t* target) {
	DWORD pID = 0;
	PROCESSENTRY32 pe32;
	pe32.dwSize = sizeof(PROCESSENTRY32);
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	do {
		if (wcscmp(pe32.szExeFile, target) == 0) {
			CloseHandle(hSnapshot);
			pID = pe32.th32ProcessID;
			break;
		}
	} while (Process32Next(hSnapshot, &pe32));

	CloseHandle(hSnapshot);
	return pID;
}

int main (int argc, char* argv[]) {
	printf("hi start\n");
	const wchar_t* process = L"PlantsVsZombies.exe";
	int pID = getProcId(process);
		
	if (pID == 0) {
		printf("Process not found.\n");
		return 1;
	}
	// HWND hGameWindow = FindWindow(NULL, L"Plants vs. Zombies");
    // DWORD pID = 0;
    // GetWindowThreadProcessId(hGameWindow, &pID);
    // HANDLE processHandle = NULL;
    // processHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pID);
    
    


	char dll[] = "test-inject.dll";		// name of dll ile to inject
	char dllPath[MAX_PATH] = { 0 };
	GetFullPathNameA(dll, MAX_PATH, dllPath, NULL);
	
	if (GetFileAttributesA(dllPath) == INVALID_FILE_ATTRIBUTES) {
		printf("DLL file not found.\n");
		return 1;
	}

	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pID);
		
	if (hProcess == NULL) {
		printf("Failed to open process.\n");
		return 1;
	}

	LPVOID pszLibFileRemote = VirtualAllocEx(hProcess, NULL, strlen(dllPath) + 1, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

	if (pszLibFileRemote == NULL) {
		printf("Failed to allocate memory in remote process.\n");
		CloseHandle(hProcess);
		return 1;
	}

	WriteProcessMemory(hProcess, pszLibFileRemote, dllPath, strlen(dllPath) + 1, NULL);
	HANDLE handleThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)LoadLibraryA, pszLibFileRemote, 0, NULL);

	WaitForSingleObject(handleThread, INFINITE);
	CloseHandle(handleThread);
	VirtualFreeEx(hProcess, dllPath, 0, MEM_RELEASE);
	CloseHandle(hProcess);

	return 0;
}