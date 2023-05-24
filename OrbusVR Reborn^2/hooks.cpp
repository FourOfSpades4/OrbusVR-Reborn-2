#include <windows.h>
#include <Minhook.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <iostream>
#include <chrono>
#include <regex>

#include "unity_strings.h"
#include "unity_data.h"
#include "socket.h"
#include "hooks.h"
#include "entity_control.h"
#include "combat_control.h"
#include "utils.h"

#include "client.h"

#include "boss.h"

std::chrono::steady_clock::time_point lastUpdateOneSecond = std::chrono::steady_clock::now();
std::chrono::steady_clock::time_point lastUpdateTenSeconds = std::chrono::steady_clock::now();
std::chrono::steady_clock::time_point lastLoginTime = std::chrono::steady_clock::now();
bool hasLoggedIn = false;

float lastPaladinShieldCooldown = -1;
bool shieldUp = false;


namespace Hooks {
    bool isGameActive(void) {
        return (std::chrono::steady_clock::now() - lastLoginTime).count() / pow(10, 9) > 5;
    }

    DWORD* (__fastcall* GetTransform)(DWORD*, DWORD*);
    DWORD* __stdcall GetTransformHook(DWORD* __this, DWORD* method) {
        return GetTransform(__this, method);
    }

    Data::Vector3(__fastcall* GetTransformPosition)(DWORD*, DWORD*);
    Data::Vector3 __stdcall GetTransformPositionHook(DWORD* __this, DWORD* method) {
        return GetTransformPosition(__this, method);
    }

    DWORD* (__fastcall* GetPlayerCharacter)(DWORD*);
    DWORD* __stdcall GetPlayerCharacterHook(DWORD* method) {
        return GetPlayerCharacter(method);
    }

    DWORD* (__fastcall* GetNetInstance)(DWORD*);
    DWORD* __stdcall GetNetInstanceHook(DWORD* method) {
        return GetNetInstance(method);
    }

    void(__fastcall* WriteString)(DWORD*, DWORD*, Data::String*, DWORD*);
    void __stdcall WriteStringHook(DWORD* __this, DWORD* writer, Data::String* str, DWORD* method) {
        WriteString(__this, writer, str, method);
    }

    void(__fastcall* TeleportMove)(DWORD*, Data::Vector3, Data::Quaternion, bool, bool, DWORD*);
    void __stdcall TeleportMoveHook(DWORD* __this, Data::Vector3 pos, Data::Quaternion rotation, bool isMoving, bool isTurning, DWORD* method) {
        TeleportMove(__this, pos, rotation, isMoving, isTurning, method);
    }

    void(__fastcall* Login)(DWORD*, int32_t, DWORD*);
    void __stdcall LoginHook(DWORD* __this, int32_t charNum, DWORD* method) {
        Login(__this, charNum, method);

        lastLoginTime = std::chrono::steady_clock::now();
        hasLoggedIn = true;
    }

    void(__fastcall* ShowActiveCharacter)(DWORD*, int32_t, DWORD*);
    void __stdcall ShowActiveCharacterHook(DWORD* __this, int32_t charNum, DWORD* method) {
        ShowActiveCharacter(__this, charNum, method);
        Login(__this, 0, method);
    }

    void(__fastcall* Resurrect)(DWORD*, DWORD*);
    void __stdcall ResurrectHook(DWORD* __this, DWORD* method) {
        Resurrect(__this, method);
    }

    void(__fastcall* DungeonShardUse)(DWORD*, DWORD*);
    void __stdcall DungeonShardUseHook(DWORD* __this, DWORD* method) {
        DungeonShardUse(__this, method);
    }

    void(__fastcall* PlayerAvatar)(DWORD*, DWORD*);
    void __stdcall PlayerAvatarHook(DWORD* __this, DWORD* method) {
        if (hasLoggedIn && (std::chrono::steady_clock::now() - lastLoginTime).count() / pow(10, 9) > 1) {
            uintptr_t playerCharacter = reinterpret_cast<uintptr_t>(GetPlayerCharacter(NULL));
            uintptr_t monster = *reinterpret_cast<uintptr_t*>(playerCharacter + 0x10 + 96);
            std::string monsterName = Data::GetString(*reinterpret_cast<Data::String**>(monster + 0x10 + 288));

            if (monsterName != "") {
                Entity::SetCurrentPlayer(monsterName);
                hasLoggedIn = false;
            }
            else {
                lastLoginTime = std::chrono::steady_clock::now();
            }
        }

        if ((std::chrono::steady_clock::now() - lastUpdateOneSecond).count() / pow(10, 9) > 1) {
            lastUpdateOneSecond = std::chrono::steady_clock::now();

            Entity::ClearUnused();

#ifdef MASTER_CLIENT
            if (Combat::isInBossCombat() && Combat::combat == Combat::BossCombat::Clockwork) {
                Boss::Clockwork::CheckForLaserDamage();
            }

            if (Combat::isInBossCombat() && Combat::combat == Combat::BossCombat::Seamstress && Combat::GetHealMult() > 0) {
                Boss::Seamstress::DealPartyDamage();
            }
#endif
            Data::String* equipment = *reinterpret_cast<Data::String**>(reinterpret_cast<uintptr_t>(__this) + 0x10 + 0x528);
            Entity::updateUnbending(Data::GetString(equipment).find("unbending") != std::string::npos);
        }

        if (Combat::isInBossCombat() && Combat::combat == Combat::BossCombat::Lizard) {
            Boss::Lizard::Update();
        }

        

#ifdef MASTER_CLIENT
        if (Combat::isInBossCombat() && Combat::combat == Combat::BossCombat::Clockwork) {
            Boss::Clockwork::CheckForLightningStrike();
        }
#endif

        if (Combat::isInBossCombat() && Combat::combat == Combat::BossCombat::PotTank) {
            Boss::PotTank::UpdatePhase();
        }
        
        PlayerAvatar(__this, method);
    }

    void(__fastcall* Monster)(DWORD*, DWORD*, DWORD*);
    void __stdcall MonsterHook(DWORD* __this, DWORD* netEntity, DWORD* method) {
        Monster(__this, netEntity, method);

        uintptr_t monsterPtr = reinterpret_cast<uintptr_t>(__this);
        Data::String* monsterName = *reinterpret_cast<Data::String**>(monsterPtr + 0x10 + 288);
        bool isPlayer = *reinterpret_cast<bool*>(monsterPtr + 0x10 + 202);
        bool isDead = *reinterpret_cast<bool*>(monsterPtr + 0x10 + 44);
        bool inCombat = *reinterpret_cast<bool*>(monsterPtr + 0x10 + 284);
        int32_t* hitPoints = reinterpret_cast<int32_t*>(monsterPtr + 0x10 + 0x108);
        int32_t maxHitPoints = *reinterpret_cast<int32_t*>(monsterPtr + 0x10 + 0x10C);
        std::string monsterNameStr = Data::GetString(monsterName);

        if (isPlayer) {
            Data::Entity player = { monsterPtr, *hitPoints, maxHitPoints, 0, Data::Color {0,0,0,0}, std::chrono::steady_clock::now() };
            Entity::AddOrUpdateEntity(monsterNameStr, player);
#ifdef MASTER_CLIENT
            Combat::UpdateCombat(inCombat);
#endif
            DWORD* healthBar = *reinterpret_cast<DWORD**>(monsterPtr + 0x10 + 0x8);

            player = Entity::GetEntity(monsterNameStr);
            Hooks::Highlight(Hooks::GetHighlighter(__this, NULL), player.color, NULL);

            if (Combat::isInBossCombat()) {
                Hooks::SetHealthBarValue(healthBar, min(*hitPoints, player.currentHealth), NULL);
                Hooks::SetHealthBarMaxValue(healthBar, maxHitPoints, NULL);
            }

#ifdef ANONYMOUS
            Hooks::SetHealthBarName(healthBar, Data::CreateString(NULL, "Anonymous", NULL), NULL);
            Hooks::SetHealthBarSurname(healthBar, Data::CreateString(NULL, "", NULL), NULL);
            Hooks::SetHealthBarSubname(healthBar, Data::CreateString(NULL, "", NULL), NULL);
#endif
            if (monsterNameStr == "FourOfSpades") {
                Hooks::SetHealthBarTitle(healthBar, Data::CreateString(NULL, "Developer", NULL), NULL);
            }
            else if (monsterNameStr == "Brettman") {
                Hooks::SetHealthBarTitle(healthBar, Data::CreateString(NULL, "Lead Tester", NULL), NULL);
            }
            else if (monsterNameStr == "Bloodshot" || monsterNameStr == "Perswon" || monsterNameStr == "CelebiBoo"
                || monsterNameStr == "Kerfuffler") {
                Hooks::SetHealthBarTitle(healthBar, Data::CreateString(NULL, "Alpha Tester", NULL), NULL);
            }
            else {
                Hooks::SetHealthBarTitle(healthBar, Data::CreateString(NULL, "Beta Tester", NULL), NULL);
            }
        }
        else {
            if (Combat::isInBossCombat()) {
                if (Combat::combat == Combat::BossCombat::Clockwork && 
                    monsterNameStr.find("Clockwork Hunter") != std::string::npos) {
                    Combat::UpdateBossPointer(monsterPtr);
                }
                if (Combat::combat == Combat::BossCombat::Broken &&
                    monsterNameStr.find("Broken Knight") != std::string::npos) {
                    Combat::UpdateBossPointer(monsterPtr);
                }
                if (Combat::combat == Combat::BossCombat::PotTank &&
                    monsterNameStr.find("Pot Tank") != std::string::npos) {
                    Combat::UpdateBossPointer(monsterPtr);
                }
                if (Combat::combat == Combat::BossCombat::Seamstress &&
                    monsterNameStr.find("Seamstress") != std::string::npos) {
                    if (maxHitPoints < 5000000) {
                        Boss::Seamstress::UpdateSeamstressAdds(true);
                    }
                    else {
                        Boss::Seamstress::UpdateSeamstressAddsExpiration();
                        Combat::UpdateBossPointer(monsterPtr);
                    }
                }
                if (Combat::combat == Combat::BossCombat::Lizard &&
                    monsterNameStr.find("Valusia Warrior") != std::string::npos) {
                    Combat::UpdateBossPointer(monsterPtr);
                }
            }
        }

        if (Combat::combat == Combat::BossCombat::PotTank && Combat::boss == monsterPtr && 
            Combat::isInBossCombat() && Boss::PotTank::IsShieldPhase()) {
            DWORD* healthBar = *reinterpret_cast<DWORD**>(monsterPtr + 0x10 + 0x8);

            Hooks::SetHealthBarValue(healthBar, Boss::PotTank::shield, NULL);
            Hooks::SetHealthBarMaxValue(healthBar, Boss::PotTank::shieldBase, NULL);
        }
    }

    void(__fastcall* ScoundrelBulletHit)(DWORD*, DWORD*, Data::Vector3*, float, int32_t, bool, DWORD*);
    void __stdcall ScoundrelBulletHitHook(DWORD* __this, DWORD* entity, Data::Vector3* vector, float speed, int32_t bullets_charged, bool fullChamber, DWORD* method) {
        ScoundrelBulletHit(__this, entity, vector, speed, bullets_charged, fullChamber, method);
    }

    void(__fastcall* SendBinaryData)(DWORD*, Data::ByteArr*, int64_t, bool, uint8_t, DWORD*);
    void __stdcall SendBinaryDataHook(DWORD* __this, Data::ByteArr* byte_arr, int64_t one, bool two, uint8_t three, DWORD* method) {
        SendBinaryData(__this, byte_arr, one, two, three, method);
    }

    void(__fastcall* UpdateHealthBar)(DWORD*, DWORD*);
    void __stdcall UpdateHealthBarHook(DWORD* __this, DWORD* method) {
        uintptr_t monster = reinterpret_cast<uintptr_t>(__this);
        bool isPlayer = *reinterpret_cast<bool*>(monster + 0x10 + 202);

        bool isDead = *reinterpret_cast<bool*>(monster + 0x10 + 44);
        int32_t hitPoints = *reinterpret_cast<int32_t*>(monster + 0x10 + 0x108);
        std::string monsterName = Data::GetString(*reinterpret_cast<Data::String**>(monster + 0x10 + 288));

        /*
        if (isPlayer && Combat::isInBossCombat() && !isDead) {
            *reinterpret_cast<int32_t*>(monster + 0x10 + 108) = 
                min(Entity::GetEntity(monsterName).currentHealth, *reinterpret_cast<int32_t*>(monster + 0x10 + 108));
        }
        */

        if (Combat::isInBossCombat() && Combat::combat == Combat::PotTank && monster == Combat::boss
            && Boss::PotTank::IsShieldPhase())
            return;
        
        if (isPlayer && Combat::isInBossCombat())
            return;
        
        if (isDead)
            return;

        UpdateHealthBar(__this, method);
    }

    void(__fastcall* SetHealthBarName)(DWORD*, Data::String*, DWORD*);
    void __stdcall SetHealthBarNameHook(DWORD* __this, Data::String* name, DWORD* method) {
        SetHealthBarName(__this, name, method);
    }

    void(__fastcall* SetHealthBarSurname)(DWORD*, Data::String*, DWORD*);
    void __stdcall SetHealthBarSurnameHook(DWORD* __this, Data::String* name, DWORD* method) {
        SetHealthBarSurname(__this, name, method);
    }

    void(__fastcall* SetHealthBarTitle)(DWORD*, Data::String*, DWORD*);
    void __stdcall SetHealthBarTitleHook(DWORD* __this, Data::String* name, DWORD* method) {
        SetHealthBarTitle(__this, name, method);
    }

    void(__fastcall* SetHealthBarSubname)(DWORD*, Data::String*, DWORD*);
    void __stdcall SetHealthBarSubnameHook(DWORD* __this, Data::String* name, DWORD* method) {
        SetHealthBarSubname(__this, name, method);
    }

    void(__fastcall* SetHealthBarValue)(DWORD*, int32_t, DWORD*);
    void __stdcall SetHealthBarValueHook(DWORD* __this, int32_t value, DWORD* method) {
        if (value < 0)
            value = 0;
        
        SetHealthBarValue(__this, value, method);
    }

    void(__fastcall* SetHealthBarMaxValue)(DWORD*, int32_t, DWORD*);
    void __stdcall SetHealthBarMaxValueHook(DWORD* __this, int32_t value, DWORD* method) {
        if (value < 1)
            value = 1;
        
        SetHealthBarMaxValue(__this, value, method);
    }

    void(__fastcall* Kill)(DWORD*, DWORD*);
    void __stdcall KillHook(DWORD* __this, DWORD* method) {
        Kill(__this, method);
    }

    void(__fastcall* StatusEffect)(DWORD*, float, int32_t, float, float, DWORD*, DWORD*, DWORD*);
    void __stdcall StatusEffectHook(DWORD* __this, float one, int32_t two, float three, float four, DWORD* sprite, DWORD* monster, DWORD* method) {
#ifdef MASTER_CLIENT
        if (two == 95) {
            if (Combat::isInBossCombat() && Combat::combat == Combat::PotTank) {
                Boss::PotTank::addStack();
            }
        }
#endif
        StatusEffect(__this, one, two, three, four, sprite, monster, method);
    }

    bool(__fastcall* HasStatusEffect)(DWORD*, int32_t, DWORD*);
    bool __stdcall HasStatusEffectHook(DWORD* __this, int32_t id, DWORD* method) {
        return HasStatusEffect(__this, id, method);
    }

    bool(__fastcall* Paladin)(DWORD*, WORD*);
    bool __stdcall PaladinHook(DWORD* __this, WORD* method) {
        uintptr_t paladin = reinterpret_cast<uintptr_t>(__this);
        uintptr_t netEntity = *reinterpret_cast<uintptr_t*>(paladin + 0x10 + 16);
        uintptr_t monster = *reinterpret_cast<uintptr_t*>(netEntity + 0x10 + 16);
        std::string monsterName = Data::GetString(*reinterpret_cast<Data::String**>(monster + 0x10 + 288));

        float damageResist = 0;

        if (*reinterpret_cast<float*>(paladin + 0x10 + 1108) < 0.999) {
            damageResist = 0.5;
        }
        if (!shieldUp) {
            Entity::UpdateEntityDamageResist(monsterName, damageResist);
        }

        if (monsterName == Entity::currentPlayer) {
            if (*reinterpret_cast<float*>(paladin + 0x10 + 920 + 20) < lastPaladinShieldCooldown) {
                Socket::SendData("3:" + monsterName + ":0.999");
                shieldUp = true;
            }

            lastPaladinShieldCooldown = *reinterpret_cast<float*>(paladin + 0x10 + 920 + 20);

            if (shieldUp && lastPaladinShieldCooldown > 1.5) {
                Socket::SendData("3:" + monsterName + ":" + std::to_string(damageResist));
                shieldUp = false;
            }
        }

        return Paladin(__this, method);
    }

    void(__fastcall* WriteToLog)(DWORD*, Data::String*, DWORD*);
    void __stdcall WriteToLogHook(DWORD* __this, Data::String* s, DWORD* method) {
        std::string str = Data::GetString(s);
        if (!Combat::isInBossCombat()) {
            if (str.find("Clockwork Hunter") != std::string::npos) {
                Socket::SendData("1:1");
                Combat::EnterCombat(1);
            }
            if (str.find("Broken Knight") != std::string::npos) {
                Socket::SendData("1:2");
                Combat::EnterCombat(2);
            }
            if (str.find("Pot Tank") != std::string::npos) {
                Socket::SendData("1:3");
                Combat::EnterCombat(3);
            }
            if (str.find("Seamstress") != std::string::npos) {
                Socket::SendData("1:4");
                Combat::EnterCombat(4);
            }
            if (str.find("Valusia Warrior") != std::string::npos) {
                Socket::SendData("1:5");
                Combat::EnterCombat(5);
            }
        }


#ifdef MASTER_CLIENT
        std::pair<std::string, int> combatData = Combat::ParseCombatText(str);
        if (Entity::IsPlayer(combatData.first) && combatData.second != 0) {

            if (Combat::isInBossCombat() && Combat::combat == Combat::Broken) {
                if (str.find("Broken Knight") != std::string::npos) {
                    Boss::Broken::SharedPainDamage(combatData.first, combatData.second);
                }
            }

            if (Combat::isInBossCombat() && Combat::combat == Combat::PotTank) {
                if (str.find("Pot Tank") != std::string::npos && Boss::PotTank::getBoost() > 0) {
                    Socket::SendData("2:" + combatData.first + ":" + std::to_string((int)(combatData.second * Boss::PotTank::getBoost())) + ":0");
                }
            }

            Socket::SendData("2:" + combatData.first + ":" + std::to_string(combatData.second) + ":0");
        }
#endif

        if (Combat::isInBossCombat() && Combat::combat == Combat::PotTank) {
            std::regex rx("Pot Tank\\(\\d*\\) took");

            std::ptrdiff_t number_of_matches = std::distance(
                std::sregex_iterator(str.begin(), str.end(), rx), std::sregex_iterator());
            if (number_of_matches) {
                if (Boss::PotTank::IsShieldPhase()) {
                    Boss::PotTank::DealShieldDamage();
                }
            }
        }

        WriteToLog(__this, s, method);
    }

    void(__fastcall* ProgressBarSetValue)(DWORD*, float, DWORD*);
    void __stdcall ProgressBarSetValueHook(DWORD* __this, float percentage, DWORD* method) {
        if (percentage < 0)
            percentage = 0;
        if (percentage > 1)
            percentage = 1;

        ProgressBarSetValue(__this, percentage, method);
    }

    void(__fastcall* ProgressBarSetColor)(DWORD*, Data::Color, DWORD*);
    void __stdcall ProgressBarSetColorHook(DWORD* __this, Data::Color color, DWORD* method) {
        ProgressBarSetColor(__this, color, method);
    }
    
    void(__fastcall* Highlight)(DWORD*, Data::Color, DWORD*);
    void __stdcall HighlightHook(DWORD* __this, Data::Color color, DWORD* method) {
        Highlight(__this, color, method);
    }

    DWORD*(__fastcall* GetHighlighter)(DWORD*, DWORD*);
    DWORD* __stdcall GetHighlighterHook(DWORD* __this, DWORD* method) {
        return GetHighlighter(__this, method);
    }

    void(__fastcall* SetHealthBarNameColor)(DWORD*, Data::Color, DWORD*);
    void __stdcall SetHealthBarNameColorHook(DWORD* __this, Data::Color color, DWORD* method) {
        SetHealthBarNameColor(__this, color, method);
    }

    Data::String* (__fastcall* GetTile)(DWORD*, uint8_t, DWORD*);
    Data::String* __stdcall GetTileHook(DWORD* __this, uint8_t id, DWORD* method) {
        return GetTile(__this, id, method);
    }
    


    void InitializeHooks(void) {
        MH_Initialize();

        uintptr_t gameAssembly = (uintptr_t)GetModuleHandleA("GameAssembly.dll");

#ifdef OCULUS
        MH_CreateHook(reinterpret_cast<LPVOID*>(gameAssembly + 17126224), &TeleportMoveHook, (LPVOID*)&TeleportMove);
        MH_EnableHook(reinterpret_cast<LPVOID*>(gameAssembly + 17126224));

        MH_CreateHook(reinterpret_cast<LPVOID*>(gameAssembly + 3165712), &LoginHook, (LPVOID*)&Login);
        MH_EnableHook(reinterpret_cast<LPVOID*>(gameAssembly + 3165712));

        MH_CreateHook(reinterpret_cast<LPVOID*>(gameAssembly + 19923808), &MonsterHook, (LPVOID*)&Monster);
        MH_EnableHook(reinterpret_cast<LPVOID*>(gameAssembly + 19923808)); // 19939232 (o) or 19923760 (c)

        MH_CreateHook(reinterpret_cast<LPVOID*>(gameAssembly + 19419840), &ScoundrelBulletHitHook, (LPVOID*)&ScoundrelBulletHit);
        MH_EnableHook(reinterpret_cast<LPVOID*>(gameAssembly + 19419840));

        MH_CreateHook(reinterpret_cast<LPVOID*>(gameAssembly + 9654112), &GetTransformHook, (LPVOID*)&GetTransform);
        MH_EnableHook(reinterpret_cast<LPVOID*>(gameAssembly + 9654112));

        MH_CreateHook(reinterpret_cast<LPVOID*>(gameAssembly + 11339824), &GetTransformPositionHook, (LPVOID*)&GetTransformPosition);
        MH_EnableHook(reinterpret_cast<LPVOID*>(gameAssembly + 11339824));

        MH_CreateHook(reinterpret_cast<LPVOID*>(gameAssembly + 17924848), &PlayerAvatarHook, (LPVOID*)&PlayerAvatar);
        MH_EnableHook(reinterpret_cast<LPVOID*>(gameAssembly + 17924848));

        MH_CreateHook(reinterpret_cast<LPVOID*>(gameAssembly + 17055424), &ResurrectHook, (LPVOID*)&Resurrect);
        MH_EnableHook(reinterpret_cast<LPVOID*>(gameAssembly + 17055424));

        MH_CreateHook(reinterpret_cast<LPVOID*>(gameAssembly + 17114736), &GetPlayerCharacterHook, (LPVOID*)&GetPlayerCharacter);
        MH_EnableHook(reinterpret_cast<LPVOID*>(gameAssembly + 17114736));

        MH_CreateHook(reinterpret_cast<LPVOID*>(gameAssembly + 17924848), &PlayerAvatarHook, (LPVOID*)&PlayerAvatar);
        MH_EnableHook(reinterpret_cast<LPVOID*>(gameAssembly + 17924848));

        MH_CreateHook(reinterpret_cast<LPVOID*>(gameAssembly + 7269456), &Data::CreateStringHook, (LPVOID*)&Data::CreateString);
        MH_EnableHook(reinterpret_cast<LPVOID*>(gameAssembly + 7269456));

        MH_CreateHook(reinterpret_cast<LPVOID*>(gameAssembly + 17709728), &WriteStringHook, (LPVOID*)&WriteString);
        MH_EnableHook(reinterpret_cast<LPVOID*>(gameAssembly + 17709728));

        MH_CreateHook(reinterpret_cast<LPVOID*>(gameAssembly + 17705504), &SendBinaryDataHook, (LPVOID*)&SendBinaryData);
        MH_EnableHook(reinterpret_cast<LPVOID*>(gameAssembly + 17705504));

        MH_CreateHook(reinterpret_cast<LPVOID*>(gameAssembly + 20702960), &SetHealthBarNameHook, (LPVOID*)&SetHealthBarName);
        MH_EnableHook(reinterpret_cast<LPVOID*>(gameAssembly + 20702960));

        MH_CreateHook(reinterpret_cast<LPVOID*>(gameAssembly + 20702976), &SetHealthBarSurnameHook, (LPVOID*)&SetHealthBarSurname);
        MH_EnableHook(reinterpret_cast<LPVOID*>(gameAssembly + 20702976));

        MH_CreateHook(reinterpret_cast<LPVOID*>(gameAssembly + 20699552), &SetHealthBarTitleHook, (LPVOID*)&SetHealthBarTitle);
        MH_EnableHook(reinterpret_cast<LPVOID*>(gameAssembly + 20699552));

        MH_CreateHook(reinterpret_cast<LPVOID*>(gameAssembly + 20702768), &SetHealthBarNameColorHook, (LPVOID*)&SetHealthBarNameColor);
        MH_EnableHook(reinterpret_cast<LPVOID*>(gameAssembly + 20702768));

        MH_CreateHook(reinterpret_cast<LPVOID*>(gameAssembly + 20690256), &SetHealthBarSubnameHook, (LPVOID*)&SetHealthBarSubname);
        MH_EnableHook(reinterpret_cast<LPVOID*>(gameAssembly + 20690256));

        MH_CreateHook(reinterpret_cast<LPVOID*>(gameAssembly + 20702992), &SetHealthBarValueHook, (LPVOID*)&SetHealthBarValue);
        MH_EnableHook(reinterpret_cast<LPVOID*>(gameAssembly + 20702992));

        MH_CreateHook(reinterpret_cast<LPVOID*>(gameAssembly + 20696000), &SetHealthBarMaxValueHook, (LPVOID*)&SetHealthBarMaxValue);
        MH_EnableHook(reinterpret_cast<LPVOID*>(gameAssembly + 20696000));

        MH_CreateHook(reinterpret_cast<LPVOID*>(gameAssembly + 19937360), &UpdateHealthBarHook, (LPVOID*)&UpdateHealthBar);
        MH_EnableHook(reinterpret_cast<LPVOID*>(gameAssembly + 19937360));

        MH_CreateHook(reinterpret_cast<LPVOID*>(gameAssembly + 17112144), &KillHook, (LPVOID*)&Kill);
        MH_EnableHook(reinterpret_cast<LPVOID*>(gameAssembly + 17112144));

        MH_CreateHook(reinterpret_cast<LPVOID*>(gameAssembly + 17709856), &WriteToLogHook, (LPVOID*)&WriteToLog);
        MH_EnableHook(reinterpret_cast<LPVOID*>(gameAssembly + 17709856));

        MH_CreateHook(reinterpret_cast<LPVOID*>(gameAssembly + 19833440), &StatusEffectHook, (LPVOID*)&StatusEffect);
        MH_EnableHook(reinterpret_cast<LPVOID*>(gameAssembly + 19833440));

        MH_CreateHook(reinterpret_cast<LPVOID*>(gameAssembly + 19925360), &HasStatusEffectHook, (LPVOID*)&HasStatusEffect);
        MH_EnableHook(reinterpret_cast<LPVOID*>(gameAssembly + 19925360));
        
        MH_CreateHook(reinterpret_cast<LPVOID*>(gameAssembly + 24199504), &PaladinHook, (LPVOID*)&Paladin);
        MH_EnableHook(reinterpret_cast<LPVOID*>(gameAssembly + 24199504));

        MH_CreateHook(reinterpret_cast<LPVOID*>(gameAssembly + 17390368), &ProgressBarSetValueHook, (LPVOID*)&ProgressBarSetValue);
        MH_EnableHook(reinterpret_cast<LPVOID*>(gameAssembly + 17390368));

        MH_CreateHook(reinterpret_cast<LPVOID*>(gameAssembly + 17389936), &ProgressBarSetColorHook, (LPVOID*)&ProgressBarSetColor);
        MH_EnableHook(reinterpret_cast<LPVOID*>(gameAssembly + 17389936));

        MH_CreateHook(reinterpret_cast<LPVOID*>(gameAssembly + 18332128), &HighlightHook, (LPVOID*)&Highlight);
        MH_EnableHook(reinterpret_cast<LPVOID*>(gameAssembly + 18332128));

        MH_CreateHook(reinterpret_cast<LPVOID*>(gameAssembly + 19938416), &GetHighlighterHook, (LPVOID*)&GetHighlighter);
        MH_EnableHook(reinterpret_cast<LPVOID*>(gameAssembly + 19938416));

        MH_CreateHook(reinterpret_cast<LPVOID*>(gameAssembly + 17929408), &GetTileHook, (LPVOID*)&GetTile);
        MH_EnableHook(reinterpret_cast<LPVOID*>(gameAssembly + 17929408));

#endif

#ifdef STEAM
        MH_CreateHook(reinterpret_cast<LPVOID*>(gameAssembly + 13790544), &TeleportMoveHook, (LPVOID*)&TeleportMove);
        MH_EnableHook(reinterpret_cast<LPVOID*>(gameAssembly + 13790544));

        MH_CreateHook(reinterpret_cast<LPVOID*>(gameAssembly + 3241984), &LoginHook, (LPVOID*)&Login);
        MH_EnableHook(reinterpret_cast<LPVOID*>(gameAssembly + 3241984));

        MH_CreateHook(reinterpret_cast<LPVOID*>(gameAssembly + 18419680), &MonsterHook, (LPVOID*)&Monster);
        MH_EnableHook(reinterpret_cast<LPVOID*>(gameAssembly + 18419680)); // 19939232 (o) or 19923760 (c)

        MH_CreateHook(reinterpret_cast<LPVOID*>(gameAssembly + 17187040), &ScoundrelBulletHitHook, (LPVOID*)&ScoundrelBulletHit);
        MH_EnableHook(reinterpret_cast<LPVOID*>(gameAssembly + 17187040));

        MH_CreateHook(reinterpret_cast<LPVOID*>(gameAssembly + 8745552), &GetTransformHook, (LPVOID*)&GetTransform);
        MH_EnableHook(reinterpret_cast<LPVOID*>(gameAssembly + 8745552));

        MH_CreateHook(reinterpret_cast<LPVOID*>(gameAssembly + 10207904), &GetTransformPositionHook, (LPVOID*)&GetTransformPosition);
        MH_EnableHook(reinterpret_cast<LPVOID*>(gameAssembly + 10207904));

        MH_CreateHook(reinterpret_cast<LPVOID*>(gameAssembly + 13401488), &PlayerAvatarHook, (LPVOID*)&PlayerAvatar);
        MH_EnableHook(reinterpret_cast<LPVOID*>(gameAssembly + 13401488));

        MH_CreateHook(reinterpret_cast<LPVOID*>(gameAssembly + 13720064), &ResurrectHook, (LPVOID*)&Resurrect);
        MH_EnableHook(reinterpret_cast<LPVOID*>(gameAssembly + 13720064));

        MH_CreateHook(reinterpret_cast<LPVOID*>(gameAssembly + 13779120), &GetPlayerCharacterHook, (LPVOID*)&GetPlayerCharacter);
        MH_EnableHook(reinterpret_cast<LPVOID*>(gameAssembly + 13779120));

        MH_CreateHook(reinterpret_cast<LPVOID*>(gameAssembly + 13401488), &PlayerAvatarHook, (LPVOID*)&PlayerAvatar);
        MH_EnableHook(reinterpret_cast<LPVOID*>(gameAssembly + 13401488));

        MH_CreateHook(reinterpret_cast<LPVOID*>(gameAssembly + 6842208), &Data::CreateStringHook, (LPVOID*)&Data::CreateString);
        MH_EnableHook(reinterpret_cast<LPVOID*>(gameAssembly + 6842208));

        MH_CreateHook(reinterpret_cast<LPVOID*>(gameAssembly + 13253824), &WriteStringHook, (LPVOID*)&WriteString);
        MH_EnableHook(reinterpret_cast<LPVOID*>(gameAssembly + 13253824));

        MH_CreateHook(reinterpret_cast<LPVOID*>(gameAssembly + 13249600), &SendBinaryDataHook, (LPVOID*)&SendBinaryData);
        MH_EnableHook(reinterpret_cast<LPVOID*>(gameAssembly + 13249600));

        MH_CreateHook(reinterpret_cast<LPVOID*>(gameAssembly + 13186048), &SetHealthBarNameHook, (LPVOID*)&SetHealthBarName);
        MH_EnableHook(reinterpret_cast<LPVOID*>(gameAssembly + 13186048));

        MH_CreateHook(reinterpret_cast<LPVOID*>(gameAssembly + 13186064), &SetHealthBarSurnameHook, (LPVOID*)&SetHealthBarSurname);
        MH_EnableHook(reinterpret_cast<LPVOID*>(gameAssembly + 13186064));

        MH_CreateHook(reinterpret_cast<LPVOID*>(gameAssembly + 13182640), &SetHealthBarTitleHook, (LPVOID*)&SetHealthBarTitle);
        MH_EnableHook(reinterpret_cast<LPVOID*>(gameAssembly + 13182640));

        MH_CreateHook(reinterpret_cast<LPVOID*>(gameAssembly + 13185856), &SetHealthBarNameColorHook, (LPVOID*)&SetHealthBarNameColor);
        MH_EnableHook(reinterpret_cast<LPVOID*>(gameAssembly + 13185856));

        MH_CreateHook(reinterpret_cast<LPVOID*>(gameAssembly + 13173280), &SetHealthBarSubnameHook, (LPVOID*)&SetHealthBarSubname);
        MH_EnableHook(reinterpret_cast<LPVOID*>(gameAssembly + 13173280));

        MH_CreateHook(reinterpret_cast<LPVOID*>(gameAssembly + 13186080), &SetHealthBarValueHook, (LPVOID*)&SetHealthBarValue);
        MH_EnableHook(reinterpret_cast<LPVOID*>(gameAssembly + 13186080));

        MH_CreateHook(reinterpret_cast<LPVOID*>(gameAssembly + 13179024), &SetHealthBarMaxValueHook, (LPVOID*)&SetHealthBarMaxValue);
        MH_EnableHook(reinterpret_cast<LPVOID*>(gameAssembly + 13179024));

        MH_CreateHook(reinterpret_cast<LPVOID*>(gameAssembly + 18433216), &UpdateHealthBarHook, (LPVOID*)&UpdateHealthBar);
        MH_EnableHook(reinterpret_cast<LPVOID*>(gameAssembly + 18433216));

        MH_CreateHook(reinterpret_cast<LPVOID*>(gameAssembly + 13776528), &KillHook, (LPVOID*)&Kill);
        MH_EnableHook(reinterpret_cast<LPVOID*>(gameAssembly + 13776528));

        MH_CreateHook(reinterpret_cast<LPVOID*>(gameAssembly + 13253952), &WriteToLogHook, (LPVOID*)&WriteToLog);
        MH_EnableHook(reinterpret_cast<LPVOID*>(gameAssembly + 13253952));

        MH_CreateHook(reinterpret_cast<LPVOID*>(gameAssembly + 16306368), &StatusEffectHook, (LPVOID*)&StatusEffect);
        MH_EnableHook(reinterpret_cast<LPVOID*>(gameAssembly + 16306368));

        MH_CreateHook(reinterpret_cast<LPVOID*>(gameAssembly + 18421232), &HasStatusEffectHook, (LPVOID*)&HasStatusEffect);
        MH_EnableHook(reinterpret_cast<LPVOID*>(gameAssembly + 18421232));

        MH_CreateHook(reinterpret_cast<LPVOID*>(gameAssembly + 21273088), &PaladinHook, (LPVOID*)&Paladin);
        MH_EnableHook(reinterpret_cast<LPVOID*>(gameAssembly + 21273088));

        MH_CreateHook(reinterpret_cast<LPVOID*>(gameAssembly + 14164640), &ProgressBarSetValueHook, (LPVOID*)&ProgressBarSetValue);
        MH_EnableHook(reinterpret_cast<LPVOID*>(gameAssembly + 14164640));

        MH_CreateHook(reinterpret_cast<LPVOID*>(gameAssembly + 14164288), &ProgressBarSetColorHook, (LPVOID*)&ProgressBarSetColor);
        MH_EnableHook(reinterpret_cast<LPVOID*>(gameAssembly + 14164288));

        MH_CreateHook(reinterpret_cast<LPVOID*>(gameAssembly + 17778512), &HighlightHook, (LPVOID*)&Highlight);
        MH_EnableHook(reinterpret_cast<LPVOID*>(gameAssembly + 17778512));

        MH_CreateHook(reinterpret_cast<LPVOID*>(gameAssembly + 18434272), &GetHighlighterHook, (LPVOID*)&GetHighlighter);
        MH_EnableHook(reinterpret_cast<LPVOID*>(gameAssembly + 18434272));

        MH_CreateHook(reinterpret_cast<LPVOID*>(gameAssembly + 13406048), &GetTileHook, (LPVOID*)&GetTile);
        MH_EnableHook(reinterpret_cast<LPVOID*>(gameAssembly + 13406048));
#endif
    }
}