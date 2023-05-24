#pragma once
#include "unity_data.h"
#include "hooks.h"

namespace Boss {
	namespace Lizard {
		void RandomizeColors(void);
		void ClearColors(void);
		void CheckForAllValidPositions(void);
		void ExitCombat(void);
		void PhaseChange(void);
		void EnterCombat(void);
		void Update(void);
	}
}