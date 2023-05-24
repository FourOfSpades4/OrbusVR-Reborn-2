#include <stdio.h>
#include <string>
#include <stdlib.h>
#include <windows.h>
#include <vector>
#include "net_manager.h"
#include "unity_strings.h"
#include "combat_control.h"
#include "entity_control.h"
#include <queue>

std::queue<std::string> requests;

bool processing = false;

static bool isNum(std::string str) {
    for (int i = 0; i < str.length(); i++) {
        if (!(isdigit(str[i]) || str[i] == '.' || str[i] == '-')) {
            return false;
        }
    }
    return true;
}

static void processDamagedEntity(std::vector<std::string> token) {
    if (token.size() >= 4) {
        std::string entity = token[1];
        if (isNum(token[2]) && isNum(token[3])) {
            int damage = std::stoi(token[2]);
            int useDamageResist = std::stoi(token[3]);
            Entity::DealDamage(entity, damage, useDamageResist);
        }
    }
}

static void processEnterCombat(std::vector<std::string> token) {
    if (token.size() >= 2) {
        Combat::EnterCombat(std::stoi(token[1]));
    }
}

static void processDamageResistEntity(std::vector<std::string> token) {
    if (token.size() >= 3) {
        std::string entity = token[1];
        if (isNum(token[2])) {
            float resist = std::stof(token[2]);
            Entity::UpdateEntityDamageResist(entity, resist);
        }
    }
}

static void processNameColorChange(std::vector<std::string> token) {
    if (token.size() >= 6) {
        std::string entity = token[1];
        if (isNum(token[2]) && isNum(token[3]) && isNum(token[4]) && isNum(token[5])) {
            Data::Color color;
            color.red = std::stof(token[2]);
            color.blue = std::stof(token[3]);
            color.green = std::stof(token[4]);
            color.alpha = std::stof(token[5]);
            Entity::SetNameColor(entity, color);
        }
    }
}

void processData(std::string allData) {
    std::cout << allData << std::endl;
    std::vector<std::string> dataList = Data::split(allData, '|');

    for (int i = 0; i < dataList.size(); i++) {
        std::string data = dataList[i];
        std::vector<std::string> token = Data::split(data, ':');

        if (token.size() == 0)
            return;
        
        if (isNum(token[0])) {
            int tokenType = std::stoi(token[0]);

            switch (tokenType) {
            case 1: processEnterCombat(token); break;
            case 2: processDamagedEntity(token); break;
            case 3: processDamageResistEntity(token); break;
            case 4: processNameColorChange(token); break;
            }
        }

    }
}

void ProcessNextData() {
    if (!processing) {
        std::string nextData = requests.front();
        requests.pop();
        processData(nextData);
    }
}

DWORD WINAPI HandleRecievedData(LPVOID lpParameter) {
    std::string* data = reinterpret_cast<std::string*>(lpParameter);
    requests.push(*data);
    delete data;

    ProcessNextData();

    return 0;
}