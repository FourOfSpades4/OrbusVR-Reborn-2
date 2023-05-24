#pragma once
#include "unity_data.h"

namespace Boss {
    namespace Clockwork {
        static Data::Vector3 laserPosition = { -1065, 0, -195 };
        bool IsInLaser(void);
        void CheckForLaserDamage(void);
        void CheckForLightningStrike(void);
        void ExitCombat(void);
    }
}