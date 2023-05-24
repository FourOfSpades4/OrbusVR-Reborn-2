#pragma once
#include <string>

DWORD WINAPI HandleRecievedData(LPVOID lpParameter);
void processData(std::string data);
void ProcessNextData(void);