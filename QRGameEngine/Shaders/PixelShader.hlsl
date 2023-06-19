struct Sprite
{
	uint index;
	float2 uv;
	float pad;
};

struct VS_OUT
{
	float4 position : SV_POSITION;
	float2 uv : uv;
	uint instance_id : instance_id;
};

struct Constants
{
	uint index;
};
ConstantBuffer<Constants> sprite_buffer_index : register(b0, space0);

SamplerState standard_sampler : register(s0);

float4 main(VS_OUT input) : SV_TARGET
{	
	StructuredBuffer<Sprite> sprite_data = ResourceDescriptorHeap[sprite_buffer_index.index];

	Texture2D colourTexture = ResourceDescriptorHeap[sprite_data[input.instance_id].index];

    return colourTexture.Sample(standard_sampler, input.uv);
}