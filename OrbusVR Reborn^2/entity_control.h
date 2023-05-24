#pragma once
#include <vector>
#include "unity_data.h"
#include <map>

namespace Entity {
	extern std::string currentPlayer;

	void ClearUnused(void);
	void AddOrUpdateEntity(std::string name, Data::Entity entity);
	void UpdateEntityHealth(std::string name, Data::Entity entity);
	void SetCurrentPlayer(std::string name);
	void UpdateHealthBar(Data::Entity entity);

	void DealDamage(std::string name, int damage, bool useDamageResist);
	void UpdateEntityDamageResist(std::string name, float damageResist);
	void RefillAllHealth(void);
	void DealDamageToAll(int damage);
	void ResetDamageResistance(void);

	void updateUnbending(bool u);
	void SetNameColor(std::string name, Data::Color color);

	std::vector <std::string> GetClosePlayers(DWORD* monster, int cutoff);

	Data::Entity GetEntity(std::string name);
	Data::Entity GetCurrentEntity(void);
	std::map<std::string, Data::Entity*> GetAllPlayers(void);
	bool IsPlayer(std::string name);

}