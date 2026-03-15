#pragma once
#include "pch.h"
#include "Common/EngineTypes.h"
#include <cmath>
#include <DirectXMath.h>

namespace MathHelp {
    Vector3 ToEulerAngles(Vector4 q) {
        q = DirectX::XMQuaternionNormalize(q);

        //Chat Gpt Version
        //float xx = q.x * q.x;
        //float yy = q.y * q.y;
        //float zz = q.z * q.z;

        //float xy = q.x * q.y;
        //float xz = q.x * q.z;
        //float yz = q.y * q.z;

        //float wx = q.w * q.x;
        //float wy = q.w * q.y;
        //float wz = q.w * q.z;
        //Vector3 chat_gpt_rotation;
        //// ----- Pitch (X axis) -----
        //float dsinp = 2.0f * (wx - yz);
        //chat_gpt_rotation.x = asinf(dsinp);

        //// Clamp for numerical safety
        //if (dsinp > 1.0f)  chat_gpt_rotation.x = DirectX::XM_PIDIV2;
        //if (dsinp < -1.0f) chat_gpt_rotation.x = -DirectX::XM_PIDIV2;


        //// ----- Roll (Z axis) -----
        //chat_gpt_rotation.z = atan2f(
        //    2.0f * (wz + xy),
        //    1.0f - 2.0f * (xx + zz)
        //);


        //// ----- Yaw (Y axis) -----
        //chat_gpt_rotation.y = atan2f(
        //    2.0f * (wy + xz),
        //    1.0f - 2.0f * (xx + yy)
        //);

        //old zxy with right handed system x = forward, y = right, z = downward
        //// pitch (x-axis rotation)
        //const double sinp = std::sqrt(1.0 + 2.0 * (q.w * q.y - q.x * q.z));
        //const double cosp = std::sqrt(1.0 - 2.0 * (q.w * q.y - q.x * q.z));
        //angles.x = 2.0f * (float)std::atan2(sinp, cosp) - DirectX::XM_PIDIV2;

        //// heading/yaw (y-axis rotation)
        //const double siny_cosp = 2.0 * (q.w * q.z + q.x * q.y);
        //const double cosy_cosp = 1.0 - 2.0 * (q.y * q.y + q.z * q.z);
        //angles.y = (float)std::atan2(siny_cosp, cosy_cosp);

        //// roll (z-axis rotation)
        //const double sinr_cosp = 2.0 * (q.w * q.x + q.y * q.z);
        //const double cosr_cosp = 1.0 - 2.0 * (q.x * q.x + q.y * q.y);
        //angles.z = (float)std::atan2(sinr_cosp, cosr_cosp);

        Vector3 euler_angles;

        // pitch (x-axis rotation)
        const double sinp = std::sqrt(1.0 + 2.0 * (q.w * q.x - q.z * q.y));
        const double cosp = std::sqrt(1.0 - 2.0 * (q.w * q.x - q.z * q.y));
        euler_angles.x = 2.0f * (float)std::atan2(sinp, cosp) - DirectX::XM_PIDIV2;

        // heading/yaw (y-axis rotation)
        const double siny_cosp = 2.0 * (q.w * q.y + q.z * q.x);
        const double cosy_cosp = 1.0 - 2.0 * (q.x * q.x + q.y * q.y);
        euler_angles.y = (float)std::atan2(siny_cosp, cosy_cosp);

        // roll (z-axis rotation)
        const double sinr_cosp = 2.0 * (q.w * q.z + q.x * q.y);
        const double cosr_cosp = 1.0 - 2.0 * (q.z * q.z + q.x * q.x);
        euler_angles.z = (float)std::atan2(sinr_cosp, cosr_cosp);

        return euler_angles;
    }
}