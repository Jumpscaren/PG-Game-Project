#include "pch.h"
#include "TransformComponent.h"
#include "SceneSystem/SceneManager.h"
#include "Scripting/CSMonoCore.h"
#include "Scripting/Objects/GameObjectInterface.h"
#include "ComponentInterface.h"
#include "Math/MathHelp.h"
#include "ParentComponent.h"
#include "Scripting/Objects/Vector2Interface.h"
#include "SceneSystem/SceneLoader.h"
#include "IO/JsonObject.h"
#include "Animation/AnimationManager.h"

MonoClassHandle TransformComponentInterface::vector2_class_handle;

DirectX::XMMATRIX GetWorldMatrix(const Vector3& position, const Vector3& rotation, const Vector3& scale)
{
	return 
		DirectX::XMMatrixScalingFromVector(scale) *
		DirectX::XMMatrixRotationRollPitchYawFromVector(rotation) *
		DirectX::XMMatrixTranslationFromVector(position);
}

TransformComponent::TransformComponent(const Vector3& position, const Vector3& rotation, const Vector3& scale) : m_scale(scale), m_rotation(rotation)
{
	world_matrix = GetWorldMatrix(position, rotation, scale);
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
	m_rotation = rotation;

	world_matrix = GetWorldMatrix(GetPosition(), m_rotation, m_scale);

	return *this;
}

TransformComponent& TransformComponent::SetScale(const Vector3& scale)
{
	m_scale = scale;

	world_matrix = GetWorldMatrix(GetPosition(), m_rotation, m_scale);

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

Vector2 TransformComponent::GetPosition2D() const
{
	return Vector2(world_matrix.r[3].m128_f32[0], world_matrix.r[3].m128_f32[1]);
}

Vector4 TransformComponent::GetRotation() const
{
	DirectX::XMVECTOR xmScale, rotationQuat, translation;
	DirectX::XMMatrixDecompose(&xmScale, &rotationQuat, &translation, world_matrix);
	return rotationQuat;
}

Vector3 TransformComponent::GetRotationEuler() const
{
	return m_rotation;
}

Vector3 TransformComponent::GetScale() const
{
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
	mono_core->HookAndRegisterMonoMethodType<TransformComponentInterface::SetLocalZIndex>(transform_class, "SetLocalZIndex", TransformComponentInterface::SetLocalZIndex);

	mono_core->HookAndRegisterMonoMethodType<TransformComponentInterface::SetScale>(transform_class, "SetScale", TransformComponentInterface::SetScale);
	mono_core->HookAndRegisterMonoMethodType<TransformComponentInterface::GetScale>(transform_class, "GetScale", TransformComponentInterface::GetScale);
	mono_core->HookAndRegisterMonoMethodType<TransformComponentInterface::SetLocalScale>(transform_class, "SetLocalScale", TransformComponentInterface::SetLocalScale);

	mono_core->HookAndRegisterMonoMethodType<TransformComponentInterface::FlipXLocally>(transform_class, "FlipXLocally", TransformComponentInterface::FlipXLocally);
	mono_core->HookAndRegisterMonoMethodType<TransformComponentInterface::FlipYLocally>(transform_class, "FlipYLocally", TransformComponentInterface::FlipYLocally);

	SceneLoader::Get()->OverrideSaveComponentMethod<TransformComponent>(SaveTransformComponent, LoadTransformComponent);

	AnimationManager::Get()->SetAnimationValue("TransformComponent", "Position", SetPositionVec2);
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

CSMonoObject TransformComponentInterface::GetPosition(SceneIndex scene_index, Entity entity)
{
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
		Vector3 old_rotation = local_transform.GetRotationEuler();
		local_transform.SetRotation(Vector3(old_rotation.x, old_rotation.y, angle));
		return;
	}
	TransformComponent& transform = entity_manager->GetComponent<TransformComponent>(entity);
	Vector3 old_rotation = transform.GetRotationEuler();
	transform.SetRotation(Vector3(old_rotation.x, old_rotation.y, angle));
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

void TransformComponentInterface::SetLocalZIndex(const CSMonoObject& object, float z_index)
{
	CSMonoObject game_object = ComponentInterface::GetGameObject(object);
	SceneIndex scene_index = GameObjectInterface::GetSceneIndex(game_object);
	Entity entity = GameObjectInterface::GetEntityID(game_object);

	EntityManager* const entity_manager = SceneManager::GetSceneManager()->GetScene(scene_index)->GetEntityManager();
	if (entity_manager->HasComponent<ParentComponent>(entity))
	{
		ParentComponent& local_transform = entity_manager->GetComponent<ParentComponent>(entity);
		local_transform.SetPositionZ(z_index);
		return;
	}
	TransformComponent& transform = entity_manager->GetComponent<TransformComponent>(entity);
	transform.SetPositionZ(z_index);
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

void TransformComponentInterface::SetLocalScale(const CSMonoObject& cs_transform, const CSMonoObject& scale)
{
	const auto game_object = ComponentInterface::GetGameObject(cs_transform);

	const SceneIndex scene_index = GameObjectInterface::GetSceneIndex(game_object);
	const Entity entity = GameObjectInterface::GetEntityID(game_object);
	const auto new_scale = Vector2Interface::GetVector2(scale);

	EntityManager* const entity_manager = SceneManager::GetSceneManager()->GetScene(scene_index)->GetEntityManager();
	if (entity_manager->HasComponent<ParentComponent>(entity))
	{
		ParentComponent& local_transform = entity_manager->GetComponent<ParentComponent>(entity);
		const auto old_scale = local_transform.GetScale();
		local_transform.SetScale(Vector3(new_scale.x, new_scale.y, old_scale.z));
		return;
	}

	TransformComponent& transform = entity_manager->GetComponent<TransformComponent>(entity);
	const auto old_scale = transform.GetScale();
	transform.SetScale(Vector3(new_scale.x, new_scale.y, old_scale.z));
}

void TransformComponentInterface::FlipXLocally(const CSMonoObject& cs_transform, const bool flip_x)
{
	//Flip around Y axis because that leads to the sprite flipping in the X axis
	const auto FlipTransformAroundXAxis = [flip_x](TransformComponent& transform) {
		Vector3 rotation = transform.GetRotationEuler();
		if (flip_x != transform.flip_x)
		{
			if (flip_x)
			{
				rotation.y = DirectX::XM_PI;
				transform.SetRotation(rotation);
			}
			else
			{
				rotation.y = 0.0f;
				transform.SetRotation(rotation);
			}
			transform.flip_x = flip_x;
		}
		};

	const auto game_object = ComponentInterface::GetGameObject(cs_transform);

	const SceneIndex scene_index = GameObjectInterface::GetSceneIndex(game_object);
	const Entity entity = GameObjectInterface::GetEntityID(game_object);

	EntityManager* const entity_manager = SceneManager::GetSceneManager()->GetScene(scene_index)->GetEntityManager();
	if (entity_manager->HasComponent<ParentComponent>(entity))
	{
		ParentComponent& local_transform = entity_manager->GetComponent<ParentComponent>(entity);
		FlipTransformAroundXAxis(local_transform);
		return;
	}

	TransformComponent& transform = entity_manager->GetComponent<TransformComponent>(entity);
	FlipTransformAroundXAxis(transform);
}

void TransformComponentInterface::FlipYLocally(const CSMonoObject& cs_transform, const bool flip_y)
{
	//Flip around X axis because that leads to the sprite flipping in the Y axis
	const auto FlipTransformAroundXAxis = [flip_y](TransformComponent& transform) {
		Vector3 rotation = transform.GetRotationEuler();
		if (flip_y != transform.flip_y)
		{
			if (flip_y)
			{
				rotation.x = DirectX::XM_PI;
				transform.SetRotation(rotation);
			}
			else
			{
				rotation.x = 0.0f;
				transform.SetRotation(rotation);
			}

			transform.flip_y = flip_y;
		}
	};

	const auto game_object = ComponentInterface::GetGameObject(cs_transform);

	const SceneIndex scene_index = GameObjectInterface::GetSceneIndex(game_object);
	const Entity entity = GameObjectInterface::GetEntityID(game_object);

	EntityManager* const entity_manager = SceneManager::GetSceneManager()->GetScene(scene_index)->GetEntityManager();
	if (entity_manager->HasComponent<ParentComponent>(entity))
	{
		ParentComponent& local_transform = entity_manager->GetComponent<ParentComponent>(entity);
		FlipTransformAroundXAxis(local_transform);
		return;
	}

	TransformComponent& transform = entity_manager->GetComponent<TransformComponent>(entity);
	FlipTransformAroundXAxis(transform);
}

PositionScaleRotation TransformComponentInterface::GetDataFromWorldMatrix(const TransformComponent& transform)
{
	DirectX::XMVECTOR xmScale, rotationQuat, translation;
	DirectX::XMMatrixDecompose(&xmScale, &rotationQuat, &translation, transform.world_matrix);

	return PositionScaleRotation{.position = translation, .scale = xmScale, .rotation = MathHelp::ToEulerAngles(rotationQuat)};
}

void TransformComponentInterface::SetPositionVec2(Entity entity, SceneIndex scene_index, const Vector2 position)
{
	EntityManager* entity_manager = SceneManager::GetEntityManager(scene_index);
	assert(entity_manager);

	if (entity_manager->HasComponent<ParentComponent>(entity))
	{
		entity_manager->GetComponent<ParentComponent>(entity).SetPosition(position);
		return;
	}

	entity_manager->GetComponent<TransformComponent>(entity).SetPosition(position);
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
