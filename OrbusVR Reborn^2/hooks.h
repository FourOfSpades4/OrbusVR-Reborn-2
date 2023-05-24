#pragma once

#include <windows.h>
#include "unity_data.h"

namespace Hooks {
    void InitializeHooks(void);
    bool isGameActive(void);

    extern DWORD* (__fastcall* GetTransform)(DWORD*, DWORD*);
    DWORD* __stdcall GetTransformHook(DWORD* __this, DWORD* method);

    extern Data::Vector3(__fastcall* GetTransformPosition)(DWORD*, DWORD*);
    Data::Vector3 __stdcall GetTransformPositionHook(DWORD* __this, DWORD* method);

    extern DWORD* (__fastcall* GetPlayerCharacter)(DWORD*);
    DWORD* __stdcall GetPlayerCharacterHook(DWORD* method);

    extern DWORD* (__fastcall* GetNetInstance)(DWORD*);
    DWORD* __stdcall GetNetInstanceHook(DWORD* method);

    extern void(__fastcall* WriteString)(DWORD*, DWORD*, Data::String*, DWORD*);
    void __stdcall WriteStringHook(DWORD* __this, DWORD* writer, Data::String* str, DWORD* method);

    extern void(__fastcall* TeleportMove)(DWORD*, Data::Vector3, Data::Quaternion, bool, bool, DWORD*);
    void __stdcall TeleportMoveHook(DWORD* __this, Data::Vector3 pos, Data::Quaternion rotation, bool isMoving, bool isTurning, DWORD* method);

    extern void(__fastcall* Login)(DWORD*, int32_t, DWORD*);
    void __stdcall LoginHook(DWORD* __this, int32_t charNum, DWORD* method);

    extern void(__fastcall* ShowActiveCharacter)(DWORD*, int32_t, DWORD*);
    void __stdcall ShowActiveCharacterHook(DWORD* __this, int32_t charNum, DWORD* method);

    extern void(__fastcall* Resurrect)(DWORD*, DWORD*);
    void __stdcall ResurrectHook(DWORD* __this, DWORD* method);

    extern void(__fastcall* DungeonShardUse)(DWORD*, DWORD*);
    void __stdcall DungeonShardUseHook(DWORD* __this, DWORD* method);

    extern void(__fastcall* PlayerAvatar)(DWORD*, DWORD*);
    void __stdcall PlayerAvatarHook(DWORD* __this, DWORD* method);

    extern void(__fastcall* Monster)(DWORD*, DWORD*, DWORD*);
    void __stdcall MonsterHook(DWORD* __this, DWORD* netEntity, DWORD* method);

    extern void(__fastcall* ScoundrelBulletHit)(DWORD*, DWORD*, Data::Vector3*, float, int32_t, bool, DWORD*);
    void __stdcall ScoundrelBulletHitHook(DWORD* __this, DWORD* entity, Data::Vector3* vector, float speed, int32_t bullets_charged, bool fullChamber, DWORD* method);

    extern void(__fastcall* SendBinaryData)(DWORD*, Data::ByteArr*, int64_t, bool, uint8_t, DWORD*);
    void __stdcall SendBinaryDataHook(DWORD* __this, Data::ByteArr* byte_arr, int64_t one, bool two, uint8_t three, DWORD* method);

    extern void(__fastcall* UpdateHealthBar)(DWORD*, DWORD*);
    void __stdcall UpdateHealthBarHook(DWORD* __this, DWORD* method);

    extern void(__fastcall* SetHealthBarName)(DWORD*, Data::String*, DWORD*);
    void __stdcall SetHealthBarNameHook(DWORD* __this, Data::String* name, DWORD* method);

    extern void(__fastcall* SetHealthBarSurname)(DWORD*, Data::String*, DWORD*);
    void __stdcall SetHealthBarSurnameHook(DWORD* __this, Data::String* name, DWORD* method);

    extern void(__fastcall* SetHealthBarTitle)(DWORD*, Data::String*, DWORD*);
    void __stdcall SetHealthBarTitleHook(DWORD* __this, Data::String* name, DWORD* method);

    extern void(__fastcall* SetHealthBarSubname)(DWORD*, Data::String*, DWORD*);
    void __stdcall SetHealthBarSubnameHook(DWORD* __this, Data::String* name, DWORD* method);

    extern void(__fastcall* SetHealthBarValue)(DWORD*, int32_t, DWORD*);
    void __stdcall SetHealthBarValueHook(DWORD* __this, int32_t value, DWORD* method);

    extern void(__fastcall* SetHealthBarMaxValue)(DWORD*, int32_t, DWORD*);
    void __stdcall SetHealthBarMaxValueHook(DWORD* __this, int32_t value, DWORD* method);

    extern void(__fastcall* Kill)(DWORD*, DWORD*);
    void __stdcall KillHook(DWORD* __this, DWORD* method);

    extern void(__fastcall* StatusEffect)(DWORD*, float, int32_t, float, float, DWORD*, DWORD*, DWORD*);
    void __stdcall StatusEffectHook(DWORD* __this, float one, int32_t two, float three, float four, DWORD* sprite, DWORD* monster, DWORD* method);

    extern bool(__fastcall* HasStatusEffect)(DWORD*, int32_t, DWORD*);
    bool __stdcall HasStatusEffectHook(DWORD* __this, int32_t id, DWORD* method);

    extern void(__fastcall* ProgressBarSetValue)(DWORD*, float, DWORD*);
    void __stdcall ProgressBarSetValueHook(DWORD* __this, float percentage, DWORD* method);

    extern void(__fastcall* ProgressBarSetColor)(DWORD*, Data::Color, DWORD*);
    void __stdcall ProgressBarSetColorHook(DWORD* __this, Data::Color color, DWORD* method);

    extern void(__fastcall* SetHealthBarNameColor)(DWORD*, Data::Color, DWORD*);
    void __stdcall SetHealthBarNameColorHook(DWORD* __this, Data::Color color, DWORD* method);

    extern void(__fastcall* Highlight)(DWORD*, Data::Color, DWORD*);
    void __stdcall HighlightHook(DWORD* __this, Data::Color color, DWORD* method);

    extern DWORD* (__fastcall* GetHighlighter)(DWORD*, DWORD*);
    DWORD* __stdcall GetHighlighterHook(DWORD* __this, DWORD* method);
}
