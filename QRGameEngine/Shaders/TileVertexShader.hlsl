struct Vertex
{
    float3 position;
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
ConstantBuffer<Constants> vertices_index : register(b0, space0);

VS_OUT main(uint vertexID : SV_VERTEXID, uint instanceID : SV_InstanceID)
{
    const uint vertex_count = 6;
    
    uint vertex_index = vertexID + instanceID * vertex_count;
    
    StructuredBuffer<Vertex> vertices = ResourceDescriptorHeap[vertices_index.index];
	
    VS_OUT output;
    output.position = float4(vertices[vertex_index].position, 1.0f);
    output.uv = vertices[vertex_index].uv;
    output.instance_id = instanceID;
    return output;
}