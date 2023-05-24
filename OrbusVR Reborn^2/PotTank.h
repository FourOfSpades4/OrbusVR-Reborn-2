#pragma once
#include "unity_data.h"


namespace Boss {
    namespace PotTank {
        extern int shield;
        extern int shieldBase;
        void UpdatePhase(void);
        void DealShieldDamage(void);
        bool IsShieldPhase(void);
        void EnterCombat(void);
        void addStack(void);
        float getBoost(void);
    }
}