struct Vertex
{
    float3 position;
    float pad;
};

struct VS_OUT
{
    float4 position : SV_POSITION;
    uint instance_id : instance_id;
};

struct Camera
{
    float4x4 view_matrix;
    float4x4 proj_matrix;
    float3 camera_position;
    float pad;
};

struct Constants
{
    uint index;
};
ConstantBuffer<Constants> vertices_index : register(b0, space0);

ConstantBuffer<Constants> camera_buffer_index : register(b1, space0);

VS_OUT main(uint vertexID : SV_VERTEXID, uint instanceID : SV_InstanceID)
{
    StructuredBuffer<Vertex> vertices = ResourceDescriptorHeap[vertices_index.index];

    VS_OUT output;
    output.position = float4(vertices[vertexID].position, 1.0f);
	
    StructuredBuffer<Camera> cameras = ResourceDescriptorHeap[camera_buffer_index.index];
    Camera camera_buffer = cameras[0];
    output.position = mul(camera_buffer.view_matrix, output.position);
    output.position = mul(camera_buffer.proj_matrix, output.position);
    
    output.instance_id = instanceID;
    return output;
}