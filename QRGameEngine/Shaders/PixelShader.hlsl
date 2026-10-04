#include "DefaultPixelBehaviour.hlsli"

float4 main(VS_OUT input) : SV_TARGET
{	
	StructuredBuffer<Sprite> sprite_data = ResourceDescriptorHeap[sprite_buffer_index.index];

    Texture2D colour_texture = ResourceDescriptorHeap[sprite_data[input.instance_id].index];
	
    float4 addative_color = sprite_data[input.instance_id].addative_color;
	
    float3 pos = input.real_position.xyz;
	
    // Force sampling from the center of the tile
    float2 uv = float2(0.875f, 0.125f);
    float4 colour = colour_texture.Sample(standard_sampler, input.uv);
    //float4 colour = colour_texture.Sample(standard_sampler, uv);
    //return colour;
	
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

    //return float4(input.uv, 0.0f, colour.w);
    //return float4(1.0f, 0.0f, 0.0f, colour.w);
    
    return float4(colour.xyz + addative_color.xyz, colour.w * addative_color.w);
	
    return float4(colour.xyz * (1.0f + specular_color + diffuse) + addative_color.xyz, colour.w * addative_color.w);
}