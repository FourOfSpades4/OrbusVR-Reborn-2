#include <chrono>
#include <map>
#include <algorithm>
#include <vector>

#include "entity_control.h"
#include "hooks.h"
#include "combat_control.h"
#include "utils.h"
#include "socket.h"

std::map<std::string, Data::Entity*> players;

bool unbending = true;
std::chrono::steady_clock::time_point lastUnbend = std::chrono::steady_clock::now();

namespace Entity {
    std::string currentPlayer;

    std::map<std::string, Data::Entity*> GetAllPlayers(void) {
        return players;
    }

    Data::Entity GetEntity(std::string name) {
        if (players.count(name) > 0) {
            return *players[name];
        }
    }

    Data::Entity GetCurrentEntity(void) {
        return *players[currentPlayer];
    }

    bool IsPlayer(std::string name) {
        return players.count(name) > 0;
    }

    void ClearUnused(void) {
        for (auto it = players.cbegin(); it != players.cend();) {
            if ((std::chrono::steady_clock::now() - it->second->lastUpdateTime).count() / pow(10, 9) > 5) {
                std::cout << "Removing Old Player: " << it->first << std::endl;
                players.erase(it++);    // or "it = m.erase(it)" since C++11
            }
            else {
                it++;
            }
        }
    }

    void SetNameColor(std::string name, Data::Color color) {
        players[name]->color = color;
    }

    void AddOrUpdateEntity(std::string name, Data::Entity entity) {
        Data::Entity* player = new Data::Entity(entity);
        if (players.count(name) == 0) {
            std::cout << "Adding New Player: " << name << std::endl;
            players.insert_or_assign(name, player);
        }
        else {
            player = players[name];
            if (entity.pointer != player->pointer) {
                player->pointer = entity.pointer;
            }
            player->lastUpdateTime = std::chrono::steady_clock::now();
        }

        UpdateEntityHealth(name, entity);
    }

    void UpdateEntityDamageResist(std::string name, float damageResist) {
        if (players.count(name) > 0) {
            players[name]->damageResist = damageResist;
        }
    }

    void RefillAllHealth(void) {
        for (std::pair<std::string, Data::Entity*> player : players) {
            player.second->currentHealth = player.second->maxHealth;
        }
    }

    void ResetDamageResistance(void) {
        for (std::pair<std::string, Data::Entity*> player : players) {
            player.second->damageResist = 0;
        }
    }

    void UpdateEntityHealth(std::string name, Data::Entity entity) {
        Data::Entity* player = players[name];
        if (!Combat::isInBossCombat()) {
            player->currentHealth = entity.currentHealth;
            player->maxHealth = entity.maxHealth;
        }
    }

    void SetCurrentPlayer(std::string name) {
        std::cout << "Assigning Player Character: " << name << std::endl;
        currentPlayer = name;
    }

    void DealDamageToAll(int damage) {
        for (std::pair<std::string, Data::Entity*> player : players) {
            Socket::SendData("2:" + player.first + ":" + std::to_string(damage) + ":0");
        }
    }

    void updateUnbending(bool u) {
        unbending = u;
    }

    void Die(void) {
        if (unbending && (std::chrono::steady_clock::now() - lastUnbend).count() / pow(10, 9) > 120) {
            lastUnbend = std::chrono::steady_clock::now();
            Socket::SendData("2:" + currentPlayer + ":" + std::to_string(GetCurrentEntity().currentHealth  - 1) + ":0");
        }
        else {
            std::cout << (std::chrono::steady_clock::now() - lastUnbend).count() / pow(10, 9) << std::endl;
            Hooks::Kill(Hooks::GetPlayerCharacter(NULL), NULL);
        }
    }

    void UpdateHealthBar(Data::Entity entity) {
        DWORD* healthBar = *reinterpret_cast<DWORD**>(entity.pointer + 0x10 + 0x8);
        DWORD* progressBar = NULL;
        if (healthBar != NULL) {
            progressBar = *reinterpret_cast<DWORD**>(reinterpret_cast<uintptr_t>(healthBar) + 0x10 + 0x20);
        }
        
        if (healthBar != NULL && progressBar != NULL) {
            Hooks::ProgressBarSetValueHook(progressBar, entity.currentHealth * 1.0 / entity.maxHealth, NULL);
        }
    }

    void PrintEntityHealth(Data::Entity entity) {
        std::cout << entity.currentHealth << "/" << entity.maxHealth << std::endl;
    }

    std::vector <std::string> GetClosePlayers(DWORD* monster, int cutoff) {
        Data::Vector3 monsterPosition = Data::GetPosition(reinterpret_cast<DWORD*>(monster));

        std::vector <std::pair<std::string, Data::Vector3>> closeVector;

        for (std::pair<std::string, Data::Entity*> player : players) {
            if (!*reinterpret_cast<bool*>(player.second->pointer + 0x10 + 44))
                closeVector.push_back(std::make_pair(player.first, Data::GetPosition(reinterpret_cast<DWORD*>(player.second->pointer))));
        }

        std::sort(closeVector.begin(), closeVector.end(),
            [monsterPosition](std::pair<std::string, Data::Vector3> a, std::pair<std::string, Data::Vector3> b)
            {
                return Data::GetDistance(monsterPosition, a.second) < Data::GetDistance(monsterPosition, b.second);
            });

        auto new_end = std::remove_if(closeVector.begin(), closeVector.end(),
            [monsterPosition, cutoff](std::pair<std::string, Data::Vector3> a)
            {
                return Data::GetDistance(monsterPosition, a.second) > cutoff;
            }
        );
        closeVector.erase(new_end, closeVector.end());

        std::vector <std::string> closePlayers;

        for (std::pair<std::string, Data::Vector3> player : closeVector) {
            closePlayers.push_back(player.first);
        }

        return closePlayers;
    }

    void DealDamage(std::string name, int damage, bool useDamageResist) {
        auto it = players.find(name);

        if (it != players.end())
        {
            Data::Entity* player = it->second;

            if (useDamageResist && damage > 0)
                damage = (int)(damage * (1 - player->damageResist));

            player->currentHealth -= damage;

            if (player->currentHealth <= 0 && name == currentPlayer && Hooks::isGameActive())
                Die();

            if (player->currentHealth > player->maxHealth)
                player->currentHealth = player->maxHealth;

            if (player->pointer != 0) {
                int32_t hitPoints = *reinterpret_cast<int32_t*>(player->pointer + 0x10 + 0x108);

                player->currentHealth = min(player->currentHealth, hitPoints);
            }

            // Hooks::UpdateHealthBarHook(reinterpret_cast<DWORD*>(player->pointer), NULL);
            // UpdateHealthBar(player);
        }
    }
}