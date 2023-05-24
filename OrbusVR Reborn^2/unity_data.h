#pragma once

#include <windows.h>
#include <iostream>
#include <chrono>

namespace Data {
    struct String {
        void* klass;
        void* method;
        int32_t length;
        uint16_t character[];
    };

    struct Vector3 {
        float x;
        float y;
        float z;
    };


    class Vector2 {
    public:
        float _x;
        float _y;

        Vector2() {
            _x = 0;
            _y = 0;
        }

        Vector2(const float x, const float y) {
            _x = x;
            _y = y;
        }

        Vector2 operator+(const Vector2& v) const {
            return Vector2(this->_x + v._x, this->_y + v._y);
        }

        Vector2 operator-(const Vector2& v) const {
            return Vector2(this->_x - v._x, this->_y - v._y);
        }

        Vector2 operator*(const float f) const {
            return Vector2(this->_x * f, this->_y * f);
        }

        float DistanceToSquared(const Vector2 p) const {
            const float dX = p._x - this->_x;
            const float dY = p._y - this->_y;

            return dX * dX + dY * dY;
        }

        float DistanceTo(const Vector2 p) const {
            return sqrt(this->DistanceToSquared(p));
        }

        float DotProduct(const Vector2 p) const {
            return this->_x * p._x + this->_y * p._y;
        }
    };

    struct Quaternion {
        float x;
        float y;
        float z;
        float w;
        bool hasValue;
    };

    struct Bounds {
        int64_t size;
        int32_t lower;
    };

    struct ByteArr {
        DWORD* obj;
        Bounds* boundsPtr;
        int64_t maxItems;
        uint8_t bytes[];
    };

    struct Color {
        float red;
        float green;
        float blue;
        float alpha;
    };

    struct Entity {
        uintptr_t pointer;
        int32_t currentHealth;
        int32_t maxHealth;
        float damageResist = 0;
        Data::Color color;
        std::chrono::steady_clock::time_point lastUpdateTime;
    };

    static Quaternion zero = { 0, 0, 0, 0, 0 };
    static Vector3 error = { -1, -1, -1 };
}
