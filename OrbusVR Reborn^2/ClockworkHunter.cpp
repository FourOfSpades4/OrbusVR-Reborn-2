#include "ClockworkHunter.h"
#include "combat_control.h"
#include "utils.h"
#include "entity_control.h"
#include "socket.h"

float laserDistance = 0.5;
float healthDamagePercent = 0.2;
bool checkStatus = true;

float lastDuration = 0;

namespace Boss {
    namespace Clockwork {
        bool IsInLaser(void) {
            Data::Vector3 bossPosition = Data::GetPosition(reinterpret_cast<DWORD*>(Combat::boss));
            Data::Vector3 playerPosition = Data::GetPosition(reinterpret_cast<DWORD*>(Entity::GetCurrentEntity().pointer));
            float distance = Data::GetClosestDistance(bossPosition.x, bossPosition.z,
                laserPosition.x, laserPosition.z, playerPosition.x, playerPosition.z);

            return distance < laserDistance;
        }

        bool IsInLaser(Data::Entity player) {
            Data::Vector3 bossPosition = Data::GetPosition(reinterpret_cast<DWORD*>(Combat::boss));
            Data::Vector3 playerPosition = Data::GetPosition(reinterpret_cast<DWORD*>(player.pointer));
            float distance = Data::GetClosestDistance(bossPosition.x, bossPosition.z,
                laserPosition.x, laserPosition.z, playerPosition.x, playerPosition.z);

            return distance < laserDistance;
        }

        void CheckForLaserDamage(void) {
            for (std::pair<std::string, Data::Entity*> pair : Entity::GetAllPlayers()) {
                Data::Entity player = *pair.second;
                if (IsInLaser(player)) {
                    if (!checkStatus ||
                        !Hooks::HasStatusEffect(reinterpret_cast<DWORD*>(player.pointer), 58, NULL)) {
                        Socket::SendData("2:" + pair.first + ":" + std::to_string((int)(player.maxHealth * healthDamagePercent)) + ":1");
                    }
                }
            }
        }

        void CheckForLightningStrike(void) {
            std::pair<std::string, float> cast = Data::GetCast(Combat::boss);
            if (lastDuration != 0 && cast.second == 0) {
                for (std::pair<std::string, Data::Entity*> pair : Entity::GetAllPlayers()) {
                    Data::Entity player = *pair.second;
                    if (!IsInLaser(player)) {
                        Socket::SendData("2:" + pair.first + ":1000000:1");
                    }
                }
            }
            lastDuration = cast.second;
        }

        void ExitCombat(void) {
            lastDuration = 0;
        }
    }
}