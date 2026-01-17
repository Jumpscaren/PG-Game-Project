struct Sprite
{
	uint index;
	float2 uv[4];
	float3 addative_color;
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

float4 main(VS_OUT input) : SV_TARGET
{	
	StructuredBuffer<Sprite> sprite_data = ResourceDescriptorHeap[sprite_buffer_index.index];

    Texture2D colour_texture = ResourceDescriptorHeap[sprite_data[input.instance_id].index];
	
    float3 addative_color = sprite_data[input.instance_id].addative_color;
	
    float3 pos = input.real_position.xyz;
	
    float4 colour = colour_texture.Sample(standard_sampler, input.uv);
	
    float3 light_dir = normalize(float3(10.0f, 10.0f, 10.0f) - pos);
    float3 view_dir = normalize(input.camera_position.xyz-pos);
	float3 halfway_dir = normalize(light_dir + view_dir);
    float3 normal = normalize(colour.xyz);
    //normal = float3(normal.x * 2.0f - 1.0f, normal.y * 2.0f - 1.0f, normal.z * 2.0f - 1.0f);
    //normal = normalize(normal);
    normal = normalize(normal * 2.0f - 1.0f);
    //return float4(normal, 1.0f);
    float spec = pow(max(dot(normal, halfway_dir), 0.0), 16.0f);
    //spec = 0.0f;
	float3 specular_color = float3(1.0f, 1.0f, 1.0f) * spec;
	
    float diff = max(dot(normal, light_dir), 0.0);
    float3 diffuse = diff * float3(1.0f, 1.0f, 1.0f) * 0.0f;

    //float alpha = colour_texture.Sample(standard_sampler, input.uv).w;
    //return float4(alpha, alpha, alpha, alpha);
	
    return float4(colour.xyz * (1.0f + specular_color + diffuse) + addative_color, colour.w);
}