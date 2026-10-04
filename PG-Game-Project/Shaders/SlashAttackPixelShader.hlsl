#include "../../QRGameEngine/Shaders/DefaultPixelBehaviour.hlsli"

float4 main(VS_OUT input) : SV_TARGET
{
    Sprite sprite = GetSprite(input.instance_id);
    Texture2D texture = GetTexture(sprite.index);
    
    float4 addative_color = sprite.addative_color;
    float4 colour = texture.Sample(standard_sampler, input.uv);
    
    return float4(1.0f, 0.0f, 0.0f, 1.0f);
    return float4(colour.xyz + addative_color.xyz, colour.w * addative_color.w);
}