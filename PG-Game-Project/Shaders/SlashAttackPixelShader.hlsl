#include "../../QRGameEngine/Shaders/DefaultPixelBehaviour.hlsli"

struct FloatConstant
{
    float value;
};
ConstantBuffer<FloatConstant> radius : register(b1, space0);

float4 main(VS_OUT input) : SV_TARGET
{
    const float2 CircleCenterPosition = float2(0.0, 0.5);
    
    Sprite sprite = GetSprite(input.instance_id);
    Texture2D texture = GetTexture(sprite.index);
    
    const float distance_squared = (input.uv.x - CircleCenterPosition.x) * (input.uv.x - CircleCenterPosition.x) + (input.uv.y - CircleCenterPosition.y) * (input.uv.y - CircleCenterPosition.y);
    if (distance_squared > radius.value * radius.value)
    {
        return float4(0.0, 0.0, 0.0, 0.0);
    }
    
    float4 addative_color = sprite.addative_color;
    float4 colour = texture.Sample(standard_sampler, input.uv);
    
    return float4(colour.xyz + addative_color.xyz, colour.w * addative_color.w);
}