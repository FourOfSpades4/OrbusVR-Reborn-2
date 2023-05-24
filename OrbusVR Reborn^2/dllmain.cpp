#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <iostream>
#include <thread>
#include <chrono>
#include <regex>
#include <list>
#include <set>
#include <fstream>
#include <string>
#include <sstream>
#include <stdlib.h>
#include <stdio.h>

#include "socket.h"
#include "hooks.h"

DWORD WINAPI Main(LPVOID lpParameter);

DWORD WINAPI Main(LPVOID lpParameter) {
    AllocConsole();
    // ShowWindow(GetConsoleWindow(), SW_HIDE);
    FILE* fout;
    FILE* fin;
    freopen_s(&fout, "CONOUT$", "w", stdout);
    freopen_s(&fin, "CONIN$", "r", stdin);

    Hooks::InitializeHooks();
    Socket::InitializeSocket();

    return 0;
}

bool OnDllProcessAttach(HMODULE module) {
    CreateThread(NULL, 0, Main, NULL, 0, NULL);

    return true;
}

bool OnDllThreadAttach(HMODULE module) {
    return true;
}

bool OnDllThreadDetach(HMODULE module) {
    return true;
}

bool OnDllProcessDetach(HMODULE module) {
    return true;
}


BOOL APIENTRY DllMain(HMODULE hModule,
    DWORD  ul_reason_for_call,
    LPVOID lpReserved
)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        return OnDllProcessAttach(hModule);
    case DLL_THREAD_ATTACH:
        return OnDllThreadAttach(hModule);
    case DLL_THREAD_DETACH:
        return OnDllThreadDetach(hModule);
    case DLL_PROCESS_DETACH:
        return OnDllProcessDetach(hModule);
    }
    return TRUE;
}

