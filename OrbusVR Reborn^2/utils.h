#pragma once

#include "hooks.h"
#include "unity_data.h"

namespace Data {
	Data::Vector3 GetPosition(DWORD* monster);

	float GetDistance(Data::Vector3 v1, Data::Vector3 v2);
	std::pair<std::string, float> GetCast(uintptr_t monster);


    float GetClosestDistance(const Data::Vector2 v, const Vector2 w,
        const Vector2 p);

    float GetClosestDistance(float segmentX1, float segmentY1,
        float segmentX2, float segmentY2, float pX,
        float pY);
}