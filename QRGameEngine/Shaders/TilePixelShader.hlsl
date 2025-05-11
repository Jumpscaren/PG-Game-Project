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

ConstantBuffer<Constants> tile_full_texture_index : register(b0, space0);
ConstantBuffer<Constants> tile_empty_texture_index : register(b1, space0);
ConstantBuffer<Constants> texture_width : register(b2, space0);
ConstantBuffer<Constants> edge_width : register(b3, space0);

SamplerState standard_sampler : register(s0);

float4 main(VS_OUT input) : SV_TARGET
{
    const float pixel_length = 1.0f / (float)texture_width.index;
    const float edge_length = pixel_length * (float)edge_width.index;
    
    /*
        16 1 32
         x---x
       8 |   | 2
         x---x
       128 4 64
    */
    
    const uint tiles[] = 
        { 
        0, // 1
        1, // 2
        1+4, // 3
        1+8, // 4
        1+8+16, // 5
        1+4+8, // 6
        1+2+4+8, // 7
        1+2+8+16, // 8
        1+4+8+16, // 9
        1+2+4+8+16, // 10
        1+2+8+16+32, // 11
        1+2+4+8+16+64, // 12
        1+2+4+8+16+128, // 13
        1+2+4+8+16+64+128, // 14
        1+2+4+8+16+32+64+128 // 15
    };
    
    uint tile_data = tiles[input.instance_id];
    
    Texture2D fill_texture = ResourceDescriptorHeap[tile_full_texture_index.index];
    float4 fill_colour = fill_texture.Sample(standard_sampler, input.uv);
    Texture2D empty_texture = ResourceDescriptorHeap[tile_empty_texture_index.index];
    float4 empty_colour = empty_texture.Sample(standard_sampler, input.uv);
    
    if ((tile_data & 1) == 1 && input.uv.x > edge_length && input.uv.x < 1.0f - edge_length && input.uv.y < edge_length)
    {
        return empty_colour;
    }
    
    if ((tile_data & 2) == 2 && input.uv.y > edge_length && input.uv.y < 1.0f - edge_length && input.uv.x > 1.0f - edge_length)
    {
        return empty_colour;
    }
    
    if ((tile_data & 4) == 4 && input.uv.x > edge_length && input.uv.x < 1.0f - edge_length && input.uv.y > 1.0f - edge_length)
    {
        return empty_colour;
    }
    
    if ((tile_data & 8) == 8 && input.uv.y > edge_length && input.uv.y < 1.0f - edge_length && input.uv.x < edge_length)
    {
        return empty_colour;
    }
    
    if ((tile_data & 16) == 16 && input.uv.x < edge_length && input.uv.y < edge_length)
    {
        return empty_colour;
    }
    
    if ((tile_data & 32) == 32 && input.uv.x > 1.0f - edge_length && input.uv.y < edge_length)
    {
        return empty_colour;
    }
    
    if ((tile_data & 64) == 64 && input.uv.x > 1.0f - edge_length && input.uv.y > 1.0f - edge_length)
    {
        return empty_colour;
    }
    
    if ((tile_data & 128) == 128 && input.uv.x < edge_length && input.uv.y > 1.0f - edge_length)
    {
        return empty_colour;
    }
    
    return fill_colour;
}