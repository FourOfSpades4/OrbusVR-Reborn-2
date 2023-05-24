#pragma once

#include <string.h>
#include "unity_data.h"
#include <windows.h>
#include <vector>

namespace Data {
	extern Data::String* (__fastcall* CreateString)(Data::String*, const char*, DWORD*);
	Data::String* __stdcall CreateStringHook(Data::String* __this, const char* name, DWORD* method);

	std::vector<std::string> split(const std::string& s, const char delim);

	std::string GetString(Data::String * str);
}