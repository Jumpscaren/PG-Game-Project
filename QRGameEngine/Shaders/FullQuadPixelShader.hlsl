struct VS_OUT
{
    float4 position : SV_POSITION;
    float2 uv : uv;
};

struct Constants
{
    uint index;
};

ConstantBuffer<Constants> texture_index : register(b0, space0);

SamplerState standard_sampler : register(s0);

float4 main(VS_OUT input) : SV_TARGET
{
    Texture2D colour_texture = ResourceDescriptorHeap[texture_index.index];
	
    float4 colour = colour_texture.Sample(standard_sampler, input.uv);
    //return float4(input.uv, 0.0f, 1.0f);
    return colour;
    return float4(colour.w, colour.w, colour.w, 1.0f);
}