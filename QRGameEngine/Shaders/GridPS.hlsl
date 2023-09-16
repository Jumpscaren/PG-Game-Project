struct VS_OUT
{
    float4 position : SV_POSITION;
    uint instance_id : instance_id;
};

struct Constants
{
    uint index;
};

struct LineColor
{
    float4 color;
};

ConstantBuffer<Constants> color_index : register(b0, space0);

float4 main(VS_OUT input) : SV_TARGET
{
    StructuredBuffer<LineColor> line_color = ResourceDescriptorHeap[color_index.index];
    
    return line_color[0].color;
    //return float4(1.0f, 1.0f, 1.0f, 1.0f);
}