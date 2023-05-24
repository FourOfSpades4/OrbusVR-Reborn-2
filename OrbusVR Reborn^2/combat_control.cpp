#include "combat_control.h"
#include "entity_control.h"
#include "boss.h"
#include <string>
#include <iostream>
#include <regex>
#include "socket.h"

namespace Combat {
	BossCombat combat;
	uintptr_t boss;
	float antiHeal = 1;
	std::chrono::steady_clock::time_point lastCombatUpdate = std::chrono::steady_clock::now();

	bool isInBossCombat(void) {
		return combat != None;
	}
	void EnterCombat(int bossId) {
		Entity::ResetDamageResistance();
		if (bossId == 0) {
			std::cout << "Exiting Combat" << std::endl;
			Entity::RefillAllHealth();
			// Entity::UpdateHealthBar(Entity::GetCurrentEntity());
			boss = 0;

			Boss::Clockwork::ExitCombat();
			Boss::Seamstress::ExitCombat();
			Boss::Lizard::ExitCombat();
		}
		else {
			std::cout << "Entering Combat: " << bossId << std::endl;
			// Entity::UpdateHealthBar(Entity::GetCurrentEntity());
		}
		switch (bossId) {
			case 0: combat = None; break;
			case 1: combat = Clockwork; break;
			case 2: combat = Broken; break;
			case 3: Boss::PotTank::EnterCombat(); combat = PotTank;  break;
			case 4: combat = Seamstress; break;
			case 5: Boss::Lizard::EnterCombat();  combat = Lizard; break;
		}
	}

	void UpdateBossPointer(uintptr_t boss) {
		if (Combat::boss != boss) {
			std::cout << "Updating Boss Pointer!" << std::endl;
		}
		Combat::boss = boss;
	}

	void SetHealMult(float a) {
		antiHeal = a;
	}

	void UpdateCombat(bool inCombat) {
		if (!inCombat) {
			if (isInBossCombat()) {
				if ((std::chrono::steady_clock::now() - lastCombatUpdate).count() / pow(10, 9) > 1) {
					Socket::SendData("1:0");
				}
			}
		}
		else {
			lastCombatUpdate = std::chrono::steady_clock::now();
		}
	}

	float GetHealMult() {
		return antiHeal;
	}

	std::pair<std::string, int> ParseCombatText(std::string str) {
		std::string name = "";
		int damage = 0;

		if (!(str.find("[Combat]") == std::string::npos ||
			str.find("took") == std::string::npos ||
			str.find("damage") == std::string::npos)) {

			size_t index = str.find("took");

			if (index != std::string::npos) {
				damage = std::stoi(str.substr(index + 4, str.find("damage") - index - 4));

				if (damage < 0)
					damage = damage * antiHeal;
				
			}

			size_t combatIndex = str.find("[Combat]");
			if (index != std::string::npos) {
				name = str.substr(combatIndex + 8, index - combatIndex - 8);
				name = std::regex_replace(name, std::regex("^ +| +$|( ) +"), "$1");
			}
		}

		return std::make_pair(name, damage);
	}
}