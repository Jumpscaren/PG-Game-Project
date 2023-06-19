struct VS_OUT
{
    float4 position : SV_POSITION;
    uint instance_id : instance_id;
};

struct Constants
{
    uint index;
};

float4 main(VS_OUT input) : SV_TARGET
{
    return float4(1.0f, 1.0f, 1.0f, 1.0f);
}