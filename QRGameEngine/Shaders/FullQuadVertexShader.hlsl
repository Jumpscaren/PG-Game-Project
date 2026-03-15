struct Vertex
{
    float3 position;
    uint uv_index;
};

struct VS_OUT
{
    float4 position : SV_POSITION;
    float2 uv : uv;
};

struct Constants
{
    uint index;
};
ConstantBuffer<Constants> vertices_index : register(b0, space0);

VS_OUT main(uint vertexID : SV_VERTEXID, uint instanceID : SV_InstanceID)
{
    StructuredBuffer<Vertex> vertices = ResourceDescriptorHeap[vertices_index.index];
    
    const float2 uv[4] =
    {
        float2(0.0f, 0.0f),
	    float2(1.0f, 0.0f),
	    float2(0.0f, 1.0f),
	    float2(1.0f, 1.0f)
    };
	
    VS_OUT output;
    output.position = float4(vertices[vertexID].position, 1.0f);
	
    output.uv = uv[vertices[vertexID].uv_index];
    return output;
}