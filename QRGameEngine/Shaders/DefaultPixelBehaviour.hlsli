struct Sprite
{
    uint index;
    float2 uv[4];
    float4 addative_color;
    float3 pad;
};

struct VS_OUT
{
    float4 position : SV_POSITION;
    float2 uv : uv;
    uint instance_id : instance_id;
    float3 real_position : real_position;
    float3 camera_position : camera_position;
    float3 light_position : light_position;
};

struct Constants
{
    uint index;
};
ConstantBuffer<Constants> sprite_buffer_index : register(b0, space0);

SamplerState standard_sampler : register(s0);

//Use only once per pixel shader execution (for performance reasons)
Sprite GetSprite(uint instance_id)
{
    StructuredBuffer<Sprite> sprite_data = ResourceDescriptorHeap[sprite_buffer_index.index];
    return sprite_data[instance_id];
}

//Use only once per pixel shader execution (for performance reasons)
Texture2D GetTexture(uint texture_index)
{
    return ResourceDescriptorHeap[texture_index];
}