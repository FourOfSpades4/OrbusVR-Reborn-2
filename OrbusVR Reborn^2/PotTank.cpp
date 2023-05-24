#include "PotTank.h"
#include "unity_data.h"
#include "combat_control.h"
#include "socket.h"
#include "client.h"
#include "entity_control.h"

std::chrono::steady_clock::time_point lastPhaseChange = std::chrono::steady_clock::now();
bool hasShield = false;
float boost = 0;

namespace Boss {
    namespace PotTank {
        int shieldBase = 500;
        int shield = shieldBase;

        void UpdatePhase(void) {
            if ((std::chrono::steady_clock::now() - lastPhaseChange).count() / pow(10, 9) > 30) {
                std::cout << "Updating Pot Tank Phase..." << std::endl;
                lastPhaseChange = std::chrono::steady_clock::now();
                std::cout << shield << std::endl;
                if (hasShield) {
#ifdef MASTER_CLIENT
                    if (shield > 0) {
                        Entity::DealDamageToAll(1000000);
                    }
#endif
                }
                else {
                    shield = shieldBase;
                }
                hasShield = !hasShield;
            }
        }
        void DealShieldDamage(void) {
            if (shield > 0)
                shield--;
        }
        bool IsShieldPhase(void) {
            return hasShield;
        }
        void addStack(void) {
            boost += 0.2;
        }
        float getBoost(void) {
            return boost;
        }
        void EnterCombat(void) {
            if (!Combat::isInBossCombat()) {
                lastPhaseChange = std::chrono::steady_clock::now();
                hasShield = false;
                boost = 0;
            }
        }
    }
}