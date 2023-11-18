struct Vertex
{
	float3 position;
    uint uv_index;
};

struct VS_OUT
{
	float4 position : SV_POSITION;
	float2 uv : uv;
	uint instance_id : instance_id;
};

struct Transform
{
	float4x4 transform;
};

struct Sprite
{
    uint index;
    float2 uv[4];
    float pad[3];
};

struct Camera
{
    float4x4 view_matrix;
    float4x4 proj_matrix;
};

//StructuredBuffer<Vertex> vertices : register(t0, space0);

struct Constants
{
	uint index;
};
ConstantBuffer<Constants> vertices_index : register(b0, space0);
//ConstantBuffer<Constants> world_matrix_buffer_index : register(b1, space0);
ConstantBuffer<Constants> transform_buffer_index : register(b1, space0);
ConstantBuffer<Constants> sprite_buffer_index : register(b1, space1);

ConstantBuffer<Constants> camera_buffer_index : register(b2, space0);

VS_OUT main(uint vertexID : SV_VERTEXID, uint instanceID : SV_InstanceID)
{
	StructuredBuffer<Vertex> vertices = ResourceDescriptorHeap[vertices_index.index];
	//StructuredBuffer<Transform> world_matrix = ResourceDescriptorHeap[world_matrix_buffer_index.index];
	StructuredBuffer<Transform> transforms = ResourceDescriptorHeap[transform_buffer_index.index];
    StructuredBuffer<Sprite> sprites = ResourceDescriptorHeap[sprite_buffer_index.index];
	
	VS_OUT output;
    output.position = mul(transforms[instanceID].transform, float4(vertices[vertexID].position, 1.0f));
	
    StructuredBuffer<Camera> cameras = ResourceDescriptorHeap[camera_buffer_index.index];
    Camera camera_buffer = cameras[0];
    output.position = mul(camera_buffer.view_matrix, output.position);
	output.position = mul(camera_buffer.proj_matrix, output.position);
	
    output.uv = sprites[instanceID].uv[vertices[vertexID].uv_index];
    output.instance_id = instanceID;
	return output;
}