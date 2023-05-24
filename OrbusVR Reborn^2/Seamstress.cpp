#pragma once
#include "unity_data.h"
#include "Seamstress.h"
#include "combat_control.h"
#include "socket.h"
#include "entity_control.h"

std::chrono::steady_clock::time_point lastAddUpdate = std::chrono::steady_clock::now();

namespace Boss {
    namespace Seamstress {
        void UpdateSeamstressAdds(bool adds) {
            if (adds) {
                lastAddUpdate = std::chrono::steady_clock::now();
                Combat::SetHealMult(-1);
            }
            else {
                Combat::SetHealMult(1);
            }
        }
        void UpdateSeamstressAddsExpiration(void) {
            if ((std::chrono::steady_clock::now() - lastAddUpdate).count() / pow(10, 9) > 1) {
                UpdateSeamstressAdds(false);
            }
        }
        void ExitCombat(void) {
            UpdateSeamstressAdds(false);
        }
        void DealPartyDamage(void) {
            Entity::DealDamageToAll(10000);
        }
    }
}