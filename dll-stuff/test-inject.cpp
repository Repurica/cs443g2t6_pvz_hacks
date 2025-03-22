#include <Windows.h>
#include <cstdio>

DWORD WINAPI MainThread(LPVOID param) {
	while (true) {
		if (GetAsyncKeyState(VK_F6) & 0x80000) {
			MessageBoxA(NULL, "F6 pressed!", "F6 pressed! caption", MB_OK);
		}
		Sleep(100);
	}
	return 0;
}

bool WINAPI DllMain(HINSTANCE hModule, DWORD dwReason, LPVOID lpReserved) {
	if (dwReason == DLL_PROCESS_ATTACH) {
		printf("DLL injected! from dll\n");
		// AllocConsole();

		MessageBoxA(NULL, "DLL injected!", "DLL injected!", MB_OK);
		CreateThread(0, 0, MainThread, hModule, 0, 0);
	}
	return true;
}