#include "unity_strings.h"
#include <sstream>

std::string Data::GetString(Data::String* str) {

    if (reinterpret_cast<uintptr_t>(str) > 100) {
        char* s = (char*)malloc(str->length + 1);

        for (int i = 0; i < str->length; i++) {
            s[i] = (char)str->character[i];
        }

        s[str->length] = '\0';
        std::string str = std::string(s);

        free(s);

        return str;
    }
    else {
        return "";
    }
}

Data::String* (__fastcall* Data::CreateString)(Data::String*, const char*, DWORD*);
Data::String* __stdcall Data::CreateStringHook(Data::String* __this, const char* name, DWORD* method) {
    return CreateString(__this, name, method);
}

std::vector<std::string> Data::split(const std::string& s, const char delim) {
    std::vector<std::string> result;
    std::stringstream ss(s);
    std::string item;

    while (getline(ss, item, delim)) {
        result.push_back(item);
    }

    return result;
}