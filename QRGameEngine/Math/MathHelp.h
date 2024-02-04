#pragma once
#include "pch.h"
#include "Common/EngineTypes.h"
#include <cmath>

namespace MathHelp {
       Vector3 ToEulerAngles(const Vector4& q) {
        Vector3 angles;

        // roll (x-axis rotation)
        const double sinr_cosp = 2.0 * (q.w * q.x + q.y * q.z);
        const double cosr_cosp = 1.0 - 2.0 * (q.x * q.x + q.y * q.y);
        angles.y = (float)std::atan2(sinr_cosp, cosr_cosp);

        // pitch (y-axis rotation)
        const double sinp = std::sqrt(1.0 + 2.0 * (q.w * q.y - q.x * q.z));
        const double cosp = std::sqrt(1.0 - 2.0 * (q.w * q.y - q.x * q.z));
        angles.x = 2.0f * (float)std::atan2(sinp, cosp) - DirectX::XM_PIDIV2;

        // yaw (z-axis rotation)
        const double siny_cosp = 2.0 * (q.w * q.z + q.x * q.y);
        const double cosy_cosp = 1.0 - 2.0 * (q.y * q.y + q.z * q.z);
        angles.z = (float)std::atan2(siny_cosp, cosy_cosp);

        return angles;
    }
}