#include <Windows.h>
#include <stdio.h>
// #include <TlHelp32.h>
// #include <iostream>

DWORD WINAPI threadFunc(LPVOID lpParam) {
    printf("Thread is running \n");
    return 0;
}

// DWORD GetProcessByName(const char* lpProcessName)
// {
//     char lpCurrentProcessName[255];

//     PROCESSENTRY32 ProcList {};
//     ProcList.dwSize = sizeof(ProcList);

//     const HANDLE hProcList = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
//     if (hProcList == INVALID_HANDLE_VALUE)
//         return -1;

//     if (!Process32First(hProcList, &ProcList))
//         return -1;

//     wcstombs_s(nullptr, lpCurrentProcessName, ProcList.szExeFile, 255);

//     if (lstrcmpA(lpCurrentProcessName, lpProcessName) == 0)
//         return ProcList.th32ProcessID;

//     while (Process32Next(hProcList, &ProcList))
//     {
//         wcstombs_s(nullptr, lpCurrentProcessName, ProcList.szExeFile, 255);

//         if (lstrcmpA(lpCurrentProcessName, lpProcessName) == 0)
//             return ProcList.th32ProcessID;
//     }

//     return -1;
// }

int main (int argc, char **argv) {
    if (argc != 3) {
		printf("usage: dll-injector.exe <path-to-dll> <PID>\n");
		return 1;
	}
	PCSTR dll_path = argv[1];
	DWORD PID = atoi(argv[2]);
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, PID);
	if (hProcess == NULL) {
		printf("Failed to retrieve handle to remote process: %d\n", GetLastError());
		return 1;
	}
	LPVOID allocated_mem = VirtualAllocEx(hProcess, NULL, strlen(dll_path) + 1, (MEM_COMMIT | MEM_RESERVE), PAGE_READWRITE);
	if (allocated_mem == NULL) {
		printf("Failed to allocated memory in remote process: %d\n", GetLastError());
		return 1;
	}
	printf("memory allocated at: %p\n", allocated_mem);
	WriteProcessMemory(hProcess, allocated_mem, dll_path, strlen(dll_path) + 1, NULL);
	HMODULE kernel32Base = GetModuleHandleW(L"kernel32.dll");
	if (kernel32Base == NULL) {
		printf("Failed to retrieve handle to kernel32.dll: %d\n", GetLastError());
		return 1;
	}
	FARPROC load_library_address = GetProcAddress(kernel32Base, "LoadLibraryA");
	HANDLE hThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)load_library_address, allocated_mem, 0, NULL);
	if (hThread == NULL) {
		printf("Failed to create thread in remote process: %d\n", GetLastError());
		return 1;
	}
	WaitForSingleObject(hThread, INFINITE);
	CloseHandle(hProcess);

	return 0;
}

// compile with 
// g++ 