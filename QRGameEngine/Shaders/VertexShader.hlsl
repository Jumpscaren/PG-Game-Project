#include "DefaultVertexBehaviour.hlsli"

VS_OUT main(uint vertexID : SV_VERTEXID, uint instanceID : SV_InstanceID)
{ 
    return DefaultVertexBehaviour(vertexID, instanceID);
}