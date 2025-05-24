#include "pch.h"
#include "TransformComponent.h"
#include "SceneSystem/SceneManager.h"
#include "Scripting/CSMonoCore.h"
#include "Scripting/Objects/GameObjectInterface.h"
#include "Input/Mouse.h"
#include "ComponentInterface.h"
#include "Math/MathHelp.h"
#include "ParentComponent.h"
#include "Scripting/Objects/Vector2Interface.h"
#include "SceneSystem/SceneLoader.h"
#include "IO/JsonObject.h"
#include "Animation/AnimationManager.h"

MonoClassHandle TransformComponentInterface::vector2_class_handle;

TransformComponent::TransformComponent(const Vector3& position, const Vector3& rotation, const Vector3& scale) : m_scale(scale), m_rotation(rotation)
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
	//DirectX::XMVECTOR pos, quat, scl;

	//DirectX::XMMatrixDecompose(&scl, &quat, &pos, world_matrix);

	m_rotation = rotation;

	world_matrix = DirectX::XMMatrixScalingFromVector(m_scale) *
		DirectX::XMMatrixRotationRollPitchYawFromVector(rotation) *
		DirectX::XMMatrixTranslationFromVector(GetPosition());

	return *this;
}

TransformComponent& TransformComponent::SetScale(const Vector3& scale)
{
	//DirectX::XMVECTOR pos, quat, scl;

	//DirectX::XMMatrixDecompose(&scl, &quat, &pos, world_matrix);

	//world_matrix = DirectX::XMMatrixScalingFromVector(scale) *
	//	DirectX::XMMatrixRotationQuaternion(quat) *
	//	DirectX::XMMatrixTranslationFromVector(pos);

	m_scale = scale;

	world_matrix = DirectX::XMMatrixScalingFromVector(scale) *
		DirectX::XMMatrixRotationRollPitchYawFromVector(m_rotation) *
		DirectX::XMMatrixTranslationFromVector(GetPosition());

	return *this;
}

TransformComponent& TransformComponent::SetPositionZ(float z)
{
	Vector3 position = GetPosition();
	position.z = z;
	SetPosition(position);

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

Vector3 TransformComponent::GetRotationEuler() const
{
	//return MathHelp::ToEulerAngles(GetRotation());
	return m_rotation;
}

Vector3 TransformComponent::GetScale() const
{
	//DirectX::XMVECTOR xmScale, rotationQuat, translation;
	//DirectX::XMMatrixDecompose(&xmScale, &rotationQuat, &translation, world_matrix);
	//return xmScale;
	return m_scale;
}

void TransformComponentInterface::RegisterInterface(CSMonoCore* mono_core)
{
	auto transform_class = mono_core->RegisterMonoClass("ScriptProject.Engine", "Transform");

	vector2_class_handle = mono_core->RegisterMonoClass("ScriptProject.EngineMath", "Vector2");

	mono_core->HookAndRegisterMonoMethodType<TransformComponentInterface::AddTransformComponent>(transform_class, "InitComponent", TransformComponentInterface::AddTransformComponent);
	mono_core->HookAndRegisterMonoMethodType<TransformComponentInterface::HasComponent>(transform_class, "HasComponent", TransformComponentInterface::HasComponent);
	mono_core->HookAndRegisterMonoMethodType<TransformComponentInterface::RemoveTransformComponent>(transform_class, "RemoveComponent", TransformComponentInterface::RemoveTransformComponent);

	mono_core->HookAndRegisterMonoMethodType<TransformComponentInterface::SetPosition>(transform_class, "SetPosition_Extern", TransformComponentInterface::SetPosition);
	mono_core->HookAndRegisterMonoMethodType<TransformComponentInterface::GetPosition>(transform_class, "GetPosition_Extern", TransformComponentInterface::GetPosition);
	mono_core->HookAndRegisterMonoMethodType<TransformComponentInterface::SetZIndex>(transform_class, "SetZIndex", TransformComponentInterface::SetZIndex);
	mono_core->HookAndRegisterMonoMethodType<TransformComponentInterface::GetZIndex>(transform_class, "GetZIndex", TransformComponentInterface::GetZIndex);
	mono_core->HookAndRegisterMonoMethodType<TransformComponentInterface::SetLocalPosition>(transform_class, "SetLocalPosition_Extern", TransformComponentInterface::SetLocalPosition);
	mono_core->HookAndRegisterMonoMethodType<TransformComponentInterface::GetLocalPosition>(transform_class, "GetLocalPosition", TransformComponentInterface::GetLocalPosition);
	mono_core->HookAndRegisterMonoMethodType<TransformComponentInterface::SetLocalRotation>(transform_class, "SetLocalRotation_Extern", TransformComponentInterface::SetLocalRotation);
	mono_core->HookAndRegisterMonoMethodType<TransformComponentInterface::GetLocalRotation>(transform_class, "GetLocalRotation_Extern", TransformComponentInterface::GetLocalRotation);

	mono_core->HookAndRegisterMonoMethodType<TransformComponentInterface::SetScale>(transform_class, "SetScale", TransformComponentInterface::SetScale);
	mono_core->HookAndRegisterMonoMethodType<TransformComponentInterface::GetScale>(transform_class, "GetScale", TransformComponentInterface::GetScale);

	SceneLoader::Get()->OverrideSaveComponentMethod<TransformComponent>(SaveTransformComponent, LoadTransformComponent);

	AnimationManager::Get()->SetAnimationValue("TransformComponent", "Scale", SetScaleVec2);
}

void TransformComponentInterface::AddTransformComponent(const CSMonoObject& object, SceneIndex scene_index, Entity entity)
{
	SceneManager::GetSceneManager()->GetScene(scene_index)->GetEntityManager()->AddComponent<TransformComponent>(entity);
}

bool TransformComponentInterface::HasComponent(const CSMonoObject& object, SceneIndex scene_index, Entity entity)
{
	return SceneManager::GetSceneManager()->GetScene(scene_index)->GetEntityManager()->HasComponent<TransformComponent>(entity);
}

void TransformComponentInterface::RemoveTransformComponent(const CSMonoObject& object, SceneIndex scene_index, Entity entity)
{
	SceneManager::GetSceneManager()->GetScene(scene_index)->GetEntityManager()->RemoveComponent<TransformComponent>(entity);
}

void TransformComponentInterface::SaveTransformComponent(Entity ent, EntityManager* entman, JsonObject* json_object)
{
	const TransformComponent& component = entman->GetComponent<TransformComponent>(ent);
	json_object->SetData((char*)&(component.world_matrix), sizeof(DirectX::XMMATRIX), "world_matrix");
	json_object->SetData(component.m_scale, "scale");
	json_object->SetData(component.m_rotation, "rotation");
}

void TransformComponentInterface::LoadTransformComponent(Entity ent, EntityManager* entman, JsonObject* json_object)
{
	if (json_object->ObjectExist("b_size") && json_object->ObjectExist("b_data"))
	{
		const uint32_t component_data_max_size = 2000;
		char component_data[component_data_max_size];
		uint32_t component_size;
		json_object->LoadData(component_size, "b_size");
		//assert(component_size <= component_data_max_size);
		json_object->LoadData(component_data, component_size, "b_data");
		DirectX::XMMATRIX* matrix = reinterpret_cast<DirectX::XMMATRIX*>(component_data);

		DirectX::XMVECTOR pos, quat, scl;
		DirectX::XMMatrixDecompose(&scl, &quat, &pos, *matrix);

		Vector3 scale = scl;
		Vector3 rotation = MathHelp::ToEulerAngles(quat);

		memcpy(component_data + sizeof(DirectX::XMMATRIX), &scale, sizeof(Vector3));
		memcpy(component_data + sizeof(DirectX::XMMATRIX) + sizeof(Vector3), &rotation, sizeof(Vector3));

		entman->SetComponentData(ent, "TransformComponent", component_data);

		return;
	}

	TransformComponent& component = entman->GetComponent<TransformComponent>(ent);
	json_object->LoadData((char*)&(component.world_matrix), sizeof(DirectX::XMMATRIX), "world_matrix");
	json_object->LoadData(component.m_scale, "scale");
	json_object->LoadData(component.m_rotation, "rotation");
}

void TransformComponentInterface::SetPosition(SceneIndex scene_index, Entity entity, float x, float y)
{
	EntityManager* const entity_manager = SceneManager::GetSceneManager()->GetScene(scene_index)->GetEntityManager();
	entity_manager->GetComponent<TransformComponent>(entity).SetPosition(Vector2(x,y));
}

CSMonoObject TransformComponentInterface::GetPosition(SceneIndex scene_index, Entity entity)//const CSMonoObject& cs_transform)
{
	//CSMonoObject game_object;
	//CSMonoCore::Get()->GetValue(game_object, cs_transform, "game_object");

	//Vector3 position = SceneManager::GetSceneManager()->GetScene(GameObjectInterface::GetSceneIndex(game_object))->GetEntityManager()->GetComponent<TransformComponent>(GameObjectInterface::GetEntityID(game_object)).GetPosition();
	const Vector3 position = SceneManager::GetSceneManager()->GetScene(scene_index)->GetEntityManager()->GetComponent<TransformComponent>(entity).GetPosition();

	CSMonoObject vector2_position(CSMonoCore::Get(), vector2_class_handle);
	CSMonoCore::Get()->SetValue(position.x, vector2_position, "x");
	CSMonoCore::Get()->SetValue(position.y, vector2_position, "y");

	return vector2_position;
}

void TransformComponentInterface::SetZIndex(const CSMonoObject& object, float z_index)
{
	CSMonoObject game_object = ComponentInterface::GetGameObject(object);

	SceneIndex scene_index = GameObjectInterface::GetSceneIndex(game_object);
	Entity entity = GameObjectInterface::GetEntityID(game_object);

	TransformComponent& transform = SceneManager::GetEntityManager(scene_index)->GetComponent<TransformComponent>(entity);
	Vector3 pos = transform.GetPosition();
	pos.z = z_index;
	transform.SetPosition(pos);
}

float TransformComponentInterface::GetZIndex(const CSMonoObject& object)
{
	CSMonoObject game_object = ComponentInterface::GetGameObject(object);

	SceneIndex scene_index = GameObjectInterface::GetSceneIndex(game_object);
	Entity entity = GameObjectInterface::GetEntityID(game_object);

	return SceneManager::GetEntityManager(scene_index)->GetComponent<TransformComponent>(entity).GetPosition().z;
}

void TransformComponentInterface::SetLocalPosition(const SceneIndex scene_index, const Entity entity, const float x, const float y)
{
	EntityManager* const entity_manager = SceneManager::GetSceneManager()->GetScene(scene_index)->GetEntityManager();
	if (entity_manager->HasComponent<ParentComponent>(entity))
	{
		entity_manager->GetComponent<ParentComponent>(entity).SetPosition(Vector2(x, y));
		return;
	}
	entity_manager->GetComponent<TransformComponent>(entity).SetPosition(Vector2(x, y));
}

CSMonoObject TransformComponentInterface::GetLocalPosition(const CSMonoObject& cs_transform)
{
	CSMonoObject game_object;
	CSMonoCore::Get()->GetValue(game_object, cs_transform, "game_object");
	const Entity entity = GameObjectInterface::GetEntityID(game_object);

	Vector3 position;
	EntityManager* const entity_manager = SceneManager::GetSceneManager()->GetScene(GameObjectInterface::GetSceneIndex(game_object))->GetEntityManager();
	if (entity_manager->HasComponent<ParentComponent>(entity))
	{
		position = std::move(entity_manager->GetComponent<ParentComponent>(entity).GetPosition());
	}
	else
	{
		position = std::move(entity_manager->GetComponent<TransformComponent>(entity).GetPosition());
	}

	CSMonoObject vector2_position(CSMonoCore::Get(), vector2_class_handle);
	CSMonoCore::Get()->SetValue(position.x, vector2_position, "x");
	CSMonoCore::Get()->SetValue(position.y, vector2_position, "y");

	return vector2_position;
}

void TransformComponentInterface::SetLocalRotation(const SceneIndex scene_index, const Entity entity, const float angle)
{
	EntityManager* const entity_manager = SceneManager::GetSceneManager()->GetScene(scene_index)->GetEntityManager();
	if (entity_manager->HasComponent<ParentComponent>(entity))
	{
		ParentComponent& local_transform = entity_manager->GetComponent<ParentComponent>(entity);
		local_transform.SetRotation(Vector3(0, 0, angle));
		return;
	}
	TransformComponent& transform = entity_manager->GetComponent<TransformComponent>(entity);
	transform.SetRotation(Vector3(0, 0, angle));
}

float TransformComponentInterface::GetLocalRotation(const SceneIndex scene_index, const Entity entity)
{
	EntityManager* const entity_manager = SceneManager::GetSceneManager()->GetScene(scene_index)->GetEntityManager();
	if (entity_manager->HasComponent<ParentComponent>(entity))
	{
		return entity_manager->GetComponent<ParentComponent>(entity).GetRotationEuler().z;
	}
	return entity_manager->GetComponent<TransformComponent>(entity).GetRotationEuler().z;
}

void TransformComponentInterface::SetScale(const CSMonoObject& cs_transform, const CSMonoObject& scale)
{
	const auto game_object = ComponentInterface::GetGameObject(cs_transform);

	const SceneIndex scene_index = GameObjectInterface::GetSceneIndex(game_object);
	const Entity entity = GameObjectInterface::GetEntityID(game_object);
	const auto new_scale = Vector2Interface::GetVector2(scale);

	EntityManager* const entity_manager = SceneManager::GetSceneManager()->GetScene(scene_index)->GetEntityManager();
	const auto old_scale = entity_manager->GetComponent<TransformComponent>(entity).GetScale();
	entity_manager->GetComponent<TransformComponent>(entity).SetScale(Vector3(new_scale.x, new_scale.y, old_scale.z));
}

CSMonoObject TransformComponentInterface::GetScale(const CSMonoObject& cs_transform)
{
	const auto game_object = ComponentInterface::GetGameObject(cs_transform);

	const SceneIndex scene_index = GameObjectInterface::GetSceneIndex(game_object);
	const Entity entity = GameObjectInterface::GetEntityID(game_object);

	EntityManager* const entity_manager = SceneManager::GetSceneManager()->GetScene(scene_index)->GetEntityManager();
	const auto scale = entity_manager->GetComponent<TransformComponent>(entity).GetScale();

	CSMonoObject vector2_scale(CSMonoCore::Get(), vector2_class_handle);
	CSMonoCore::Get()->SetValue(scale.x, vector2_scale, "x");
	CSMonoCore::Get()->SetValue(scale.y, vector2_scale, "y");
	return vector2_scale;
}

PositionScaleRotation TransformComponentInterface::GetDataFromWorldMatrix(const TransformComponent& transform)
{
	DirectX::XMVECTOR xmScale, rotationQuat, translation;
	DirectX::XMMatrixDecompose(&xmScale, &rotationQuat, &translation, transform.world_matrix);
	return PositionScaleRotation{.position = translation, .scale = xmScale, .rotation = MathHelp::ToEulerAngles(rotationQuat)};
}

void TransformComponentInterface::SetScaleVec2(Entity entity, SceneIndex scene_index, Vector2 scale)
{
	EntityManager* entity_manager = SceneManager::GetEntityManager(scene_index);
	assert(entity_manager);

	if (entity_manager->HasComponent<ParentComponent>(entity))
	{
		ParentComponent& parent = entity_manager->GetComponent<ParentComponent>(entity);
		Vector3 old_scale = parent.GetScale();
		old_scale.x = scale.x;
		old_scale.y = scale.y;
		parent.SetScale(old_scale);
		return;
	}

	TransformComponent& transform = entity_manager->GetComponent<TransformComponent>(entity);
	Vector3 old_scale = transform.GetScale();
	old_scale.x = scale.x;
	old_scale.y = scale.y;
	transform.SetScale(old_scale);
}
