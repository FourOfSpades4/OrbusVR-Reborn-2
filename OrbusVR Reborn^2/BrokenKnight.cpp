#include "BrokenKnight.h"
#include "combat_control.h"
#include "utils.h"
#include "entity_control.h"
#include "socket.h"

namespace Boss {
    namespace Broken {
        bool SharedPainDamage(std::string name, int damage) {
            damage += 30000;

            std::vector<std::string> nearPlayers = 
                Entity::GetClosePlayers(reinterpret_cast<DWORD*>(Entity::GetEntity(name).pointer), 10);
            if (nearPlayers.size() == 1) {
                std::cout << "One Player: " << name << std::endl;
                Socket::SendData("2:" + name + ":" + std::to_string(damage * 2) + ":0");
            }
            if (nearPlayers.size() == 2) {
                std::cout << "Two Players: " << nearPlayers[0] << " " << nearPlayers[1] << std::endl;
                Socket::SendData("2:" + nearPlayers[0] + ":" + std::to_string((int)(damage * 0.5)) + ":0");
                Socket::SendData("2:" + nearPlayers[1] + ":" + std::to_string((int)(damage * 1.5)) + ":0");
            }
            if (nearPlayers.size() >= 3) {
                std::cout << "Three Players: " << nearPlayers[1] << " " << nearPlayers[2] << std::endl;
                Socket::SendData("2:" + nearPlayers[1] + ":" + std::to_string(damage) + ":0");
                Socket::SendData("2:" + nearPlayers[2] + ":" + std::to_string(damage) + ":0");
            }
        }
    }
}