#include "pch.h"
#include "TransformComponent.h"
#include "SceneSystem/SceneManager.h"
#include "Scripting/CSMonoCore.h"
#include "Scripting/Objects/GameObjectInterface.h"

MonoClassHandle TransformComponentInterface::vector2_class_handle;

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

TransformComponent& TransformComponent::SetPosition(const Vector2& position)
{
	world_matrix.r[3].m128_f32[0] = position.x;
	world_matrix.r[3].m128_f32[1] = position.y;

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

	vector2_class_handle = mono_core->RegisterMonoClass("ScriptProject.Math", "Vector2");

	mono_core->HookAndRegisterMonoMethodType<TransformComponentInterface::SetPosition>(transform_class, "SetPosition_Extern", TransformComponentInterface::SetPosition);
	mono_core->HookAndRegisterMonoMethodType<TransformComponentInterface::AddTransformComponent>(transform_class, "InitComponent", TransformComponentInterface::AddTransformComponent);
	mono_core->HookAndRegisterMonoMethodType<TransformComponentInterface::GetPosition>(transform_class, "GetPosition", TransformComponentInterface::GetPosition);
}

void TransformComponentInterface::AddTransformComponent(CSMonoObject object, SceneIndex scene_index, Entity entity)
{
	SceneManager::GetSceneManager()->GetScene(scene_index)->GetEntityManager()->AddComponent<TransformComponent>(entity);
}

void TransformComponentInterface::SetPosition(SceneIndex scene_index, Entity entity, float x, float y)
{
	SceneManager::GetSceneManager()->GetScene(scene_index)->GetEntityManager()->GetComponent<TransformComponent>(entity).SetPosition(Vector2(x,y));
}

CSMonoObject TransformComponentInterface::GetPosition(CSMonoObject cs_transform)
{
	CSMonoObject game_object;
	CSMonoCore::Get()->GetValue(game_object, cs_transform, "game_object");

	Vector3 position = SceneManager::GetSceneManager()->GetScene(GameObjectInterface::GetSceneIndex(game_object))->GetEntityManager()->GetComponent<TransformComponent>(GameObjectInterface::GetEntityID(game_object)).GetPosition();

	CSMonoObject vector2_position(CSMonoCore::Get(), vector2_class_handle);
	CSMonoCore::Get()->SetValue(position.x, vector2_position, "x");
	CSMonoCore::Get()->SetValue(position.y, vector2_position, "y");

	return vector2_position;
}
