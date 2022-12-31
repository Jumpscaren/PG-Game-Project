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

struct Transform
{
	float4x4 transform;
};

//StructuredBuffer<Vertex> vertices : register(t0, space0);

struct Constants
{
	uint index;
};
ConstantBuffer<Constants> vertices_index : register(b0, space0);
//ConstantBuffer<Constants> world_matrix_buffer_index : register(b1, space0);
ConstantBuffer<Constants> transform_buffer_index : register(b1, space0);

VS_OUT main(uint vertexID : SV_VERTEXID, uint instanceID : SV_InstanceID)
{
	StructuredBuffer<Vertex> vertices = ResourceDescriptorHeap[vertices_index.index];
	//StructuredBuffer<Transform> world_matrix = ResourceDescriptorHeap[world_matrix_buffer_index.index];
	StructuredBuffer<Transform> transforms = ResourceDescriptorHeap[transform_buffer_index.index];

	VS_OUT output;
	output.position = mul(transforms[instanceID].transform, float4(vertices[vertexID].position, 1.0f));
	output.uv = vertices[vertexID].uv;
	output.instance_id = instanceID;
	return output;
}