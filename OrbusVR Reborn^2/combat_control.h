#pragma once
#include <string>

namespace Combat {
	enum BossCombat {None, Clockwork, Broken, PotTank, Seamstress, Lizard};
	bool isInBossCombat(void);
	void EnterCombat(int bossId);
	void UpdateBossPointer(uintptr_t boss);
	std::pair<std::string, int> ParseCombatText(std::string str);
	void UpdateCombat(bool inCombat);

	void SetHealMult(float a);
	float GetHealMult(void);

	extern BossCombat combat;
	extern uintptr_t boss;
}