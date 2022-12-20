struct VS_OUT
{
	float4 position : SV_POSITION;
	float2 uv : uv;
};

struct Constants
{
	uint index;
};
ConstantBuffer<Constants> textureConstant : register(b0, space0);

SamplerState standardSampler : register(s0);

float4 main(VS_OUT input) : SV_TARGET
{
	Texture2D colourTexture = ResourceDescriptorHeap[textureConstant.index];

	//return float4(1.0f, 0.0f, 0.0f, 1.0f);
	return colourTexture.Sample(standardSampler, input.uv);
}