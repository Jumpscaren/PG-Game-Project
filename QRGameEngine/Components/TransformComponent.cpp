#include "pch.h"
#include "TransformComponent.h"
#include "SceneSystem/SceneManager.h"
#include "Scripting/CSMonoCore.h"

TransformComponent::TransformComponent(const Vector3& position, const Vector3& rotation, const Vector3& scale)
{
	world_matrix = DirectX::XMMatrixScalingFromVector(scale) *
		DirectX::XMMatrixRotationRollPitchYawFromVector(rotation) *
		DirectX::XMMatrixTranslationFromVector(position);
}

TransformComponent& TransformComponent::SetPosition(const Vector3& position)
{
	world_matrix.r[3].m128_f32[0] = position.x;
	world_matrix.r[3].m128_f32[1] = position.y;
	world_matrix.r[3].m128_f32[2] = position.z;

	return *this;
}

TransformComponent& TransformComponent::SetRotation(const Vector3& rotation)
{
	DirectX::XMVECTOR pos, quat, scl;

	DirectX::XMMatrixDecompose(&scl, &quat, &pos, world_matrix);

	world_matrix = DirectX::XMMatrixScalingFromVector(scl) *
		DirectX::XMMatrixRotationRollPitchYawFromVector(rotation) *
		DirectX::XMMatrixTranslationFromVector(pos);

	return *this;
}

TransformComponent& TransformComponent::SetScale(const Vector3& scale)
{
	DirectX::XMVECTOR pos, quat, scl;

	DirectX::XMMatrixDecompose(&scl, &quat, &pos, world_matrix);

	world_matrix = DirectX::XMMatrixScalingFromVector(scale) *
		DirectX::XMMatrixRotationRollPitchYawFromVector(quat) *
		DirectX::XMMatrixTranslationFromVector(pos);

	return *this;
}

Vector3 TransformComponent::GetPosition() const
{
	return Vector3(world_matrix.r[3].m128_f32[0], world_matrix.r[3].m128_f32[1], world_matrix.r[3].m128_f32[2]);
}

Vector4 TransformComponent::GetRotation() const
{
	DirectX::XMVECTOR xmScale, rotationQuat, translation;
	DirectX::XMMatrixDecompose(&xmScale, &rotationQuat, &translation, world_matrix);
	return rotationQuat;
}

Vector3 TransformComponent::GetScale() const
{
	DirectX::XMVECTOR xmScale, rotationQuat, translation;
	DirectX::XMMatrixDecompose(&xmScale, &rotationQuat, &translation, world_matrix);
	return xmScale;
}

void TransformComponentInterface::RegisterInterface(CSMonoCore* mono_core)
{
	auto transform_class = mono_core->RegisterMonoClass("ScriptProject.Engine", "Transform");

	mono_core->HookAndRegisterMonoMethodType<TransformComponentInterface::SetPosition>(transform_class, "SetPosition_Extern", TransformComponentInterface::SetPosition);
	mono_core->HookAndRegisterMonoMethodType<TransformComponentInterface::AddTransformComponent>(transform_class, "InitComponent", TransformComponentInterface::AddTransformComponent);
}

void TransformComponentInterface::AddTransformComponent(CSMonoObject object, SceneIndex scene_index, Entity entity)
{
	SceneManager::GetSceneManager()->GetScene(scene_index)->GetEntityManager()->AddComponent<TransformComponent>(entity);
}

void TransformComponentInterface::SetPosition(SceneIndex scene_index, Entity entity, float x, float y, float z)
{
	SceneManager::GetSceneManager()->GetScene(scene_index)->GetEntityManager()->GetComponent<TransformComponent>(entity).SetPosition(Vector3(x,y,z));
}
