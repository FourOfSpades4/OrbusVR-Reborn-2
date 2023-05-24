#include "utils.h"
#include "unity_strings.h"

Data::Vector3 Data::GetPosition(DWORD* monster) {
    uintptr_t monsterPtr = reinterpret_cast<uintptr_t>(monster);
    DWORD* tranform = Hooks::GetTransform(monster, NULL);
    if (tranform != NULL) {
        return Hooks::GetTransformPosition(tranform, NULL);
    }
    else {
        return Data::error;
    }
}

float Data::GetDistance(Data::Vector3 v1, Data::Vector3 v2) {
    return sqrtf(pow(v1.x - v2.x, 2) + pow(v1.z - v2.z, 2));
}

float Data::GetClosestDistance(const Data::Vector2 v, const Vector2 w,
    const Vector2 p) {

    const float distSq = v.DistanceToSquared(w);
    if (distSq == 0.0)
        return v.DistanceTo(p);

    const float t = (p - v).DotProduct(w - v) / distSq;

    if (t < 0.0)
        return v.DistanceTo(p);
    
    else if (t > 1.0)
        return w.DistanceTo(p);

    const Vector2 projection = v + ((w - v) * t);

    return p.DistanceTo(projection);
}

float Data::GetClosestDistance(float segmentX1, float segmentY1,
    float segmentX2, float segmentY2, float pX,
    float pY) {
    Vector2 q;

    float distance = GetClosestDistance(
        Vector2(segmentX1, segmentY1),
        Vector2(segmentX2, segmentY2),
        Vector2(pX, pY));

    return distance;
}

std::pair<std::string, float> Data::GetCast(uintptr_t monster) {
    float progress = 0;
    std::string castString = "";
    if (monster != 0) {
        bool casting = *reinterpret_cast<bool*>(monster + 0x10 + 0xC0);
        if (casting) {
            uintptr_t textMeshPtr = *reinterpret_cast<uintptr_t*>(monster + 0x10 + 152);
            castString = Data::GetString(*reinterpret_cast<Data::String**>(textMeshPtr + 0x10 + 72));
            if (castString == "Creationism") {
                castString = "";
            }
            else {
                uintptr_t progressBarPtr = *reinterpret_cast<uintptr_t*>(monster + 0x10 + 144);
                progress = *reinterpret_cast<float*>(progressBarPtr + 0x10 + 0xC);
            }
        }
    }
    return std::make_pair(castString, progress);
}