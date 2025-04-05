#include <unordered_set>
#include <winsock2.h>
#include <Windows.h>
#include <basetsd.h>
#include <minwindef.h>
#include <synchapi.h>
#include <thread>
#include <debugapi.h>
#include <winnt.h>
#include <sstream>
#include <iostream>
#include <ostream>
#include <string>
#include <ws2tcpip.h>
#include <stdio.h>
#include <vector>
#include <map>
using namespace std;



DWORD string_to_hex(string i){
    stringstream stream;
    stream << hex << stoi(i);
    string result(stream.str());
    return strtol(result.insert(0,"0x").c_str(),0, 0);
}

void set_zombie_init_speed(HANDLE processHandle){
    DWORD_PTR BaseAddress;
    DWORD_PTR buffer;

    BaseAddress=0x006E1B31;
    buffer=0xD8000000003446C7;
    WriteProcessMemory(processHandle, (LPVOID)BaseAddress, &buffer, 8, nullptr);

    BaseAddress=0x006E1B39;
    buffer=0x006796B00DDC344E;
    WriteProcessMemory(processHandle, (LPVOID)BaseAddress, &buffer, 8, nullptr);
      
    BaseAddress=0x006E1B41;
    buffer=0xFFE4DA12E9;
    WriteProcessMemory(processHandle, (LPVOID)BaseAddress, &buffer, 8, nullptr);
      
    BaseAddress=0x0052F54F;
    buffer=0x1f0f001B25DDE9;
    WriteProcessMemory(processHandle, (LPVOID)BaseAddress, &buffer, 8, nullptr);
      
    BaseAddress=0x0052F556;
    buffer=0x0040;
    WriteProcessMemory(processHandle, (LPVOID)BaseAddress, &buffer, 2, nullptr);
        

}   

void set_zombie(HANDLE processHandle, DWORD type, DWORD row,DWORD speed){
    DWORD_PTR BaseAddress;
    DWORD_PTR buffer;
    BaseAddress=0x006E1A88;
    WriteProcessMemory(processHandle, (LPVOID)BaseAddress, &type, 1, nullptr);
    BaseAddress=0x006E1A8A;
    WriteProcessMemory(processHandle, (LPVOID)BaseAddress, &row, 1, nullptr);
    

    BaseAddress=0x00552244;
    buffer=0xfe;
    WriteProcessMemory(processHandle, (LPVOID)BaseAddress, &buffer, 1, nullptr);
    Sleep(20);

    
    HANDLE thread = CreateRemoteThread(processHandle, nullptr, 0, LPTHREAD_START_ROUTINE(0x6E1A72), nullptr, 0, nullptr);
    DWORD wait_status = WaitForSingleObject(thread, INFINITE);
    CloseHandle(thread);


    buffer=0xdb;
    WriteProcessMemory(processHandle, (LPVOID)BaseAddress, &buffer, 1, nullptr);
    
    
    // BaseAddress=0x006E1B34;
    // WriteProcessMemory(processHandle, (LPVOID)BaseAddress, &speed, 4, nullptr);
    
}


int get_zombie_address(HANDLE processHandle){
    DWORD_PTR baseAddress = 0x700000;
    int zombie_base_address;
    ReadProcessMemory(processHandle, (LPVOID)(baseAddress), &zombie_base_address, 4, NULL);

    return zombie_base_address;
}




void updateZombieData(HANDLE processHandle, DWORD_PTR memoAddr,float col_axis,DWORD health,int disappear,int status){
    
    WriteProcessMemory(processHandle, (LPVOID)(memoAddr+0x2c), &col_axis, 4, NULL);
    
    WriteProcessMemory(processHandle, (LPVOID)(memoAddr+0xc8), &health, 4, NULL);
    WriteProcessMemory(processHandle, (LPVOID)(memoAddr+0xEC), &disappear, 4, NULL);

    WriteProcessMemory(processHandle, (LPVOID)(memoAddr+0x28), &status, 4, NULL);

}



VOID sun(HANDLE processHandle){
    DWORD_PTR baseAddress = 0x6A9EC0;
    DWORD_PTR sun_address;
    DWORD_PTR plant_num_address;

    DWORD_PTR offset=0x768;
    int sun_num=1000;
    int plant_num;
    ReadProcessMemory(processHandle, (LPVOID)baseAddress, &baseAddress, 4, NULL);
    ReadProcessMemory(processHandle, (LPVOID)(baseAddress+offset), &baseAddress, 4, NULL);
    sun_address=baseAddress+0x5560;

    WriteProcessMemory(processHandle, (LPVOID)(sun_address), &sun_num, 4, NULL);

}




VOID clear_lawn_mowers(HANDLE processHandle){
    DWORD_PTR baseAddress=0x6A9EC0;
    DWORD_PTR offset;
    int buffer=1;
    

    ReadProcessMemory(processHandle, (LPVOID)baseAddress, &baseAddress, 4, NULL);
    offset=0x768;

    ReadProcessMemory(processHandle, (LPVOID)(baseAddress+offset), &baseAddress, 4, NULL);
    offset=0x100;
    
    ReadProcessMemory(processHandle, (LPVOID)(baseAddress+offset), &baseAddress, 4, NULL);

    WriteProcessMemory(processHandle, (LPVOID)(baseAddress+0x30), &buffer, 8, nullptr);
    WriteProcessMemory(processHandle, (LPVOID)(baseAddress+0x30+0x48), &buffer, 8, nullptr);
    WriteProcessMemory(processHandle, (LPVOID)(baseAddress+0x30+0x48+0x48), &buffer, 8, nullptr);
    WriteProcessMemory(processHandle, (LPVOID)(baseAddress+0x30+0x48+0x48+0x48), &buffer, 8, nullptr);
    WriteProcessMemory(processHandle, (LPVOID)(baseAddress+0x30+0x48+0x48+0x48+0x48), &buffer, 8, nullptr);

}


void auto_collect(HANDLE processHandle){
    
    DWORD_PTR buffer=0xeb;
    WriteProcessMemory(processHandle, (LPVOID)0x004315ef, &buffer, 1, nullptr);

}


int disable_pause_have_settings(HANDLE processHandle){
    DWORD_PTR baseAddress = 0x6A9EC0;
    DWORD_PTR offset=0;
    ReadProcessMemory(processHandle, (LPVOID)baseAddress, &baseAddress, 4, NULL);
    offset=0x768;
    ReadProcessMemory(processHandle, (LPVOID)(baseAddress+offset), &baseAddress, 4, NULL);
    offset=0x164;
    int target=0;
    while (1){
        cout << "123" << endl;
        WriteProcessMemory(processHandle, (LPVOID)(baseAddress+offset), &target, 4, NULL);
        Sleep(1);
        }
    return 0;
}


int disable_natural_zombie(HANDLE processHandle){
    DWORD_PTR baseAddress = 0x413E55;
    DWORD_PTR buffer;
    buffer = 0x90909090909090;
    WriteProcessMemory(processHandle, (LPVOID)baseAddress, &buffer, 7, NULL);
    return 0;
}


VOID set_zombie_init(HANDLE processHandle){
    DWORD_PTR BaseAddress;

    BaseAddress=0x006E1A72;
    unsigned long long buffer=0x8B006A9EC0158B60;
    WriteProcessMemory(processHandle, (LPVOID)BaseAddress, &buffer, 8, nullptr);
    
    BaseAddress=0x006E1A7A;
    buffer=0x60928B0000076892;
    WriteProcessMemory(processHandle, (LPVOID)BaseAddress, &buffer, 8, nullptr);
    
    BaseAddress=0x006E1A82;
    buffer=0x6A096A000001;

    WriteProcessMemory(processHandle, (LPVOID)BaseAddress, &buffer, 6, nullptr);
    
    BaseAddress=0x006E1A88;
    buffer=0x0C;//zombie type
    WriteProcessMemory(processHandle, (LPVOID)BaseAddress, &buffer, 1, nullptr);
    
    BaseAddress=0x006E1A89;
    buffer=0xB8;
    
    WriteProcessMemory(processHandle, (LPVOID)BaseAddress, &buffer, 1, nullptr);
    
    BaseAddress=0x006E1A8A;
    buffer=0x03;//zombie row
    WriteProcessMemory(processHandle, (LPVOID)BaseAddress, &buffer, 1, nullptr);

    BaseAddress=0x006E1A8B;
    buffer=0x000000;
    WriteProcessMemory(processHandle, (LPVOID)BaseAddress, &buffer, 3, nullptr);

    BaseAddress=0x006E1A8E;
    buffer=0x61FFD486BBE8CA8B;
    WriteProcessMemory(processHandle, (LPVOID)BaseAddress, &buffer, 8, nullptr);
    
    
    BaseAddress=0x006E1A96;
    buffer=0xC3;
    WriteProcessMemory(processHandle, (LPVOID)BaseAddress, &buffer, 1, nullptr);
    
}


void no_damage(HANDLE processHandle){
    DWORD_PTR buffer;
    buffer=0x90909090;
    WriteProcessMemory(processHandle, (LPVOID)0x0053165f, &buffer, 4, NULL);

}


int main() {

    HWND hGameWindow = FindWindow(NULL, "Plants vs. Zombies");
    DWORD pID = 0;
    GetWindowThreadProcessId(hGameWindow, &pID);
    HANDLE processHandle = NULL;
    processHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pID);
    
    cout << "Process 123Handle:1 " << hGameWindow << endl;
    


    // thread(disable_pause_have_settings,processHandle).detach();
    // disable_natural_zombie(processHandle);
    // clear_lawn_mowers(processHandle) ;
    // no_damage(processHandle);
    // auto_collect(processHandle);
    sun(processHandle);
    // set_zombie_init_speed(processHandle);


    
    while(1){
        Sleep(1000);
    }
    return 0;

}
