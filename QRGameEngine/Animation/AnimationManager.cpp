#include "pch.h"
#include "AnimationManager.h"
#include "IO/JsonObject.h"
#include "IO/OutputFile.h"
#include "ECS/EntityManager.h"
#include "Renderer/RenderCore.h"
#include "Asset/AssetManager.h"
#include "Scripting/CSMonoCore.h"
#include "Scripting/Objects/GameObjectInterface.h"
#include "Time/Time.h"
#include "SceneSystem/SceneHierarchy.h"

AnimationManager* AnimationManager::s_singleton = nullptr;

AnimationManager::AnimationManager()
{
	s_singleton = this;
}

AnimationManager* const AnimationManager::Get()
{
	return s_singleton;
}

void SetSpriteAndAnimationKeyFrames(AnimatableSpriteComponent& animatable_sprite, const ParentAnimationDataMap& parent_animation_data_map, const SceneIndex scene_index, const Entity old_parent_entity, const Entity new_parent_entity)
{
	const ParentAnimationData& parent_animation_data = parent_animation_data_map.at(old_parent_entity);
	std::vector<AnimationKeyFrameId> key_frames_indicies;
	key_frames_indicies.resize(parent_animation_data.animation_value_sections.size());
	animatable_sprite.key_frames_indicies.insert({ old_parent_entity, key_frames_indicies });

	EntityManager* ent_man = SceneManager::GetSceneManager()->GetEntityManager(scene_index);
	if (ent_man->HasComponent<SpriteComponent>(new_parent_entity))
	{
		SpriteComponent& sprite = ent_man->GetComponent<SpriteComponent>(new_parent_entity);
		if (parent_animation_data.texture_path != "")
		{
			//sprite = parent_animation_data.sprite;
			//Could potentially stop working later
			for (int i = 0; i < 4; ++i)
			{
				sprite.uv[i] = parent_animation_data.sprite.uv[i];
			}
			
			sprite.texture_handle = RenderCore::Get()->LoadTexture(parent_animation_data.texture_path, scene_index);
		}
	}

	if (SceneHierarchy::Get()->ParentHasChildren(scene_index, new_parent_entity))
	{
		const std::vector<SceneHierarchy::Child>& children = SceneHierarchy::Get()->GetOrderedChildren(scene_index, new_parent_entity);
		const std::vector<Entity>& cached_children = parent_animation_data.children;

		for (int i = 0; i < children.size() && i < cached_children.size(); ++i)
		{
			SetSpriteAndAnimationKeyFrames(animatable_sprite, parent_animation_data_map, scene_index, cached_children[i], children[i].child);
		}
	}
}

void SetEntityAnimationKeyData(AnimatableSpriteComponent& animatable_sprite, const ParentAnimationDataMap& parent_animation_data_map, ValueStorage& value_storage, const SceneIndex scene_index, const Entity old_parent_entity, const Entity new_parent_entity)
{
	const ParentAnimationData& parent_animation_data = parent_animation_data_map.at(old_parent_entity);
	std::vector<AnimationKeyFrameId>& key_frame_indicies = animatable_sprite.key_frames_indicies.at(old_parent_entity);

	for (int section_index = 0; section_index < parent_animation_data.animation_value_sections.size(); ++section_index)
	{
		const AnimationValueSection& animation_value_section = parent_animation_data.animation_value_sections[section_index];
		AnimationKeyFrameId current_key_frame = key_frame_indicies[section_index];

		for (AnimationKeyFrameId i = current_key_frame + 1; i < animation_value_section.animation_key_frames.size(); ++i)
		{
			const AnimationKeyFrame& key_frame = animation_value_section.animation_key_frames[i];
			if (animatable_sprite.current_animation_time > key_frame.timestamp)
			{
				current_key_frame = i;
			}
			else
			{
				break;
			}
		}

		key_frame_indicies[section_index] = current_key_frame;
		const AnimationKeyFrame& key_frame = animation_value_section.animation_key_frames[current_key_frame];

		if (animatable_sprite.current_animation_time + 0.00001f <= key_frame.timestamp)
		{
			continue;
		}

		AnimationManager::SetAnimationKeyData(current_key_frame, animation_value_section, value_storage, scene_index, new_parent_entity, animatable_sprite.current_animation_time);
	}

	if (SceneHierarchy::Get()->ParentHasChildren(scene_index, new_parent_entity))
	{
		const std::vector<SceneHierarchy::Child>& children = SceneHierarchy::Get()->GetOrderedChildren(scene_index, new_parent_entity);
		const std::vector<Entity>& cached_children = parent_animation_data.children;

		for (int i = 0; i < children.size() && i < cached_children.size(); ++i)
		{
			SetEntityAnimationKeyData(animatable_sprite, parent_animation_data_map, value_storage, scene_index, cached_children[i], children[i].child);
		}
	}
}

void AnimationManager::UpdateAnimations(EntityManager* ent_man)
{
	const float delta_time = (float)Time::GetDeltaTime();

	ent_man->System<AnimatableSpriteComponent>([&](Entity parent_entity, AnimatableSpriteComponent& animatable_sprite)
		{
			animatable_sprite.current_animation_time += delta_time * animatable_sprite.speed;

			if (animatable_sprite.current_animation_time > animatable_sprite.max_animation_time)
			{
				if (animatable_sprite.loop)
				{
					animatable_sprite.current_animation_time = 0.0f;
					animatable_sprite.key_frames_indicies.clear();
					const AnimationData& cached_animation_data = m_cached_animation_data[animatable_sprite.id];
					SetSpriteAndAnimationKeyFrames(animatable_sprite, m_cached_animation_data[animatable_sprite.id].parent_animation_data_map, ent_man->GetSceneIndex(), cached_animation_data.old_parent_entity, parent_entity);
				}
				else
				{
					animatable_sprite.finished = true;
					return;
				}
			}

			const ParentAnimationDataMap& parent_animation_data_map = m_cached_animation_data[animatable_sprite.id].parent_animation_data_map;
			SetEntityAnimationKeyData(animatable_sprite, parent_animation_data_map, m_animation_value_storage, ent_man->GetSceneIndex(), m_cached_animation_data[animatable_sprite.id].old_parent_entity, parent_entity);
		}
	);
}

void AnimationManager::SaveAnimation(const SceneIndex scene_index, const Entity entity, const std::string& animation_file_name)
{
	auto* ent_man = SceneManager::GetSceneManager()->GetEntityManager(scene_index);

	assert(ent_man->HasComponent<AnimatableSpriteComponent>(entity));
	assert(ent_man->HasComponent<SpriteComponent>(entity));

	JsonObject save_animation;
	JsonObject animation_data = save_animation.CreateSubJsonObject("AnimatableSpriteComponent");
	JsonObject sprite_data = save_animation.CreateSubJsonObject("SpriteComponent");

	AnimatableSpriteComponentInterface::SaveAnimatableSpriteComponent(entity, ent_man, &animation_data);
	SpriteComponentInterface::SaveSpriteComponent(entity, ent_man, &sprite_data);
	const SpriteComponent& sprite = ent_man->GetComponent<SpriteComponent>(entity);
	const std::string texture_path = AssetManager::Get()->GetAssetPath(RenderCore::Get()->GetTextureAssetHandle(sprite.texture_handle));

	sprite_data.SetData(texture_path, "Texture_Path");

	OutputFile file(animation_file_name, OutputFile::FileMode::WRITE);
	const std::string json_string = save_animation.GetJsonString();
	file.Write((uint32_t)json_string.size());
	file.Write((void*)json_string.c_str(), (uint32_t)json_string.size());
	file.Close();
}

bool AnimationManager::LoadAnimation(const SceneIndex scene_index, const Entity entity, const std::string& animation_file_name)
{
	auto* ent_man = SceneManager::GetSceneManager()->GetEntityManager(scene_index);

	assert(ent_man->HasComponent<AnimatableSpriteComponent>(entity));

	AnimatableSpriteComponent& animatable_sprite = ent_man->GetComponent<AnimatableSpriteComponent>(entity);

	const uint64_t animation_file_name_hash = std::hash<std::string>{}(animation_file_name);
	auto it = m_animation_name_to_cached_data.find(animation_file_name_hash);
	if (it != m_animation_name_to_cached_data.end())
	{
		const AnimationData& cached_animation_data = m_cached_animation_data[it->second];

		const float old_speed = animatable_sprite.speed;
		animatable_sprite = cached_animation_data.animatable_sprite_data;
		animatable_sprite.speed = old_speed;

		SetSpriteAndAnimationKeyFrames(animatable_sprite, cached_animation_data.parent_animation_data_map, scene_index, cached_animation_data.old_parent_entity, entity);

		return true;
	}

	OutputFile file(animation_file_name, OutputFile::FileMode::READ);
	if (!file.FileExists())
	{
		return false;
	}

	const uint32_t json_text_size = file.Read<uint32_t>();
	std::string json_string;
	json_string.resize(json_text_size);
	file.Read((void*)json_string.c_str(), json_text_size);
	file.Close();

	JsonObject load_animation(json_string);

	AnimationData cached_animation_data;
	JsonObject hierarchy_data = load_animation.GetSubJsonObject("HierarchyData");
	Entity old_parent_entity;
	hierarchy_data.LoadData(old_parent_entity, "ParentEntity");
	LoadChildData(hierarchy_data, old_parent_entity, ent_man, cached_animation_data.parent_animation_data_map, m_animation_value_storage);

	JsonObject sprite_data = load_animation.GetSubJsonObject("SpriteData");

	animatable_sprite.key_frames_indicies.clear();
	SetSpriteAndAnimationKeyFrames(animatable_sprite, cached_animation_data.parent_animation_data_map, scene_index, old_parent_entity, entity);

	animatable_sprite.current_animation_time = 0.0f;

	JsonObject animatable_sprite_data = load_animation.GetSubJsonObject("AnimatableSpriteData");
	AnimatableSpriteComponentInterface::LoadAnimatableSpriteComponent(entity, ent_man, &animatable_sprite_data);
	animatable_sprite_data.LoadData(animatable_sprite.max_animation_time, "AnimationTime");

	animatable_sprite.id = m_animation_id++;
	cached_animation_data.animatable_sprite_data = animatable_sprite;
	cached_animation_data.old_parent_entity = old_parent_entity;

	m_animation_name_to_cached_data.insert({ animation_file_name_hash, m_cached_animation_data.size() });
	m_cached_animation_data.push_back(cached_animation_data);

	return true;
}

std::string AnimationManager::GetAnimationTexturePath(const SceneIndex scene_index, const Entity entity, const std::string& animation_file_name) const
{
	if (!SceneManager::GetEntityManager(scene_index)->HasComponent<AnimatableSpriteComponent>(entity))
	{
		return "";
	}

	const uint64_t animation_file_name_hash = std::hash<std::string>{}(animation_file_name);
	if (!m_animation_name_to_cached_data.contains(animation_file_name_hash))
	{
		return "";

	}

	const uint64_t cached_data_index = m_animation_name_to_cached_data.at(animation_file_name_hash);
	return m_cached_animation_data[cached_data_index].texture_path;
}

void AnimationManager::RegisterInterface(CSMonoCore* mono_core)
{
	const auto animation_manager_class = mono_core->RegisterMonoClass("ScriptProject.Engine", "AnimationManager");

	mono_core->HookAndRegisterMonoMethodType<AnimationManager::SaveAnimationMono>(animation_manager_class, "SaveAnimation", AnimationManager::SaveAnimationMono);
	mono_core->HookAndRegisterMonoMethodType<AnimationManager::LoadAnimationMono>(animation_manager_class, "LoadAnimation", AnimationManager::LoadAnimationMono);
	mono_core->HookAndRegisterMonoMethodType<AnimationManager::IsAnimationPlaying>(animation_manager_class, "IsAnimationPlaying", AnimationManager::IsAnimationPlaying);
}

void AnimationManager::SaveAnimationMono(const CSMonoObject& game_object, const std::string& animation_file_name)
{
	const auto entity = GameObjectInterface::GetEntityID(game_object);
	const auto scene_index = GameObjectInterface::GetSceneIndex(game_object);
	AnimationManager::Get()->SaveAnimation(scene_index, entity, animation_file_name);
}

void AnimationManager::LoadAnimationMono(const CSMonoObject& game_object, const std::string& animation_file_name)
{
	const auto entity = GameObjectInterface::GetEntityID(game_object);
	const auto scene_index = GameObjectInterface::GetSceneIndex(game_object);
	AnimationManager::Get()->LoadAnimation(scene_index, entity, animation_file_name);
}

bool AnimationManager::IsAnimationPlaying(const CSMonoObject& game_object, const std::string& animation_file_name)
{
	const uint64_t animation_file_name_hash = std::hash<std::string>{}(animation_file_name);
	const AnimationManager* anim_manager = AnimationManager::Get();
	const auto it = anim_manager->m_animation_name_to_cached_data.find(animation_file_name_hash);
	if (it == anim_manager->m_animation_name_to_cached_data.end())
	{
		return false;
	}

	const uint64_t cached_data_index = anim_manager->m_animation_name_to_cached_data.at(animation_file_name_hash);
	return AnimatableSpriteComponentInterface::IsAnimationPlaying(game_object, anim_manager->m_cached_animation_data[cached_data_index].animatable_sprite_data.id);
}

AnimationSetterId AnimationManager::GetAnimationValueSetterStorageIndex(const std::string& component_name, const std::string& value_name) const
{
	for (int i = 0; i < m_animation_value_setters.size(); ++i)
	{
		if (m_animation_value_setters[i].component_name == component_name && m_animation_value_setters[i].value_name == value_name)
		{
			return i;
		}
	}

	return -1;
}

const AnimationValueSetterStorage& AnimationManager::GetAnimationValueSetterStorage(AnimationSetterId storage_index) const
{
	assert(m_animation_value_setters.size() > storage_index);
	return m_animation_value_setters[storage_index];
}

struct LoadSpriteData
{
	SpriteComponent sprite;
	std::string texture_path;
};

LoadSpriteData LoadSprite(JsonObject& entity_data, Entity entity, EntityManager* entity_manager)
{
	JsonObject sprite_data = entity_data.GetSubJsonObject("SpriteData");
	SpriteComponent& load_sprite = entity_manager->GetComponent<SpriteComponent>(entity);

	std::string texture_path = "";
	SpriteComponentInterface::LoadSpriteComponent(entity, entity_manager, &sprite_data);
	sprite_data.LoadData(texture_path, "Texture_Path");
	//texture_path = "../QRGameEngine/Textures/OrcCarrier.png";
	SpriteComponent sprite = load_sprite;
	if (texture_path != "")
	{
		sprite.texture_handle = RenderCore::Get()->LoadTexture(texture_path, entity_manager->GetSceneIndex());
	}

	return LoadSpriteData{.sprite = sprite, .texture_path = texture_path};
}

void AnimationManager::LoadChildData(JsonObject& parent_data, Entity parent, EntityManager* entity_manager, ParentAnimationDataMap& parent_animation_data_map, ValueStorage& storage)
{
	JsonObject new_parent_data = parent_data.GetSubJsonObject("Parent");

	ParentAnimationData parent_animation_data{};

	const Entity sprite_entity = entity_manager->NewEntity();
	entity_manager->AddComponent<SpriteComponent>(sprite_entity);

	const LoadSpriteData load_sprite_data = LoadSprite(new_parent_data, sprite_entity, entity_manager);
	parent_animation_data.sprite = load_sprite_data.sprite;
	parent_animation_data.texture_path = load_sprite_data.texture_path;

	entity_manager->RemoveEntity(sprite_entity);

	parent_animation_data.animation_value_sections = LoadAnimatableSprite(new_parent_data, entity_manager, storage);

	if (new_parent_data.ObjectExist("NoChildren"))
	{
		return;
	}

	uint32_t children_count = 0;
	new_parent_data.LoadData(children_count, "ChildrenCount");

	for (uint32_t i = 0; i < children_count; ++i)
	{
		Entity child;
		new_parent_data.LoadData(child, "Child" + std::to_string(i));
		LoadChildData(new_parent_data, child, entity_manager, parent_animation_data_map, storage);

		parent_animation_data.children.push_back(child);
	}

	parent_animation_data_map.insert({ parent, parent_animation_data });
}

void LoadAnimationValueSection(JsonObject& animation_value_section_data, AnimationValueSection& animation_section, ValueStorage& storage)
{
	int i = 0;
	for (const std::string& key_frame_name : animation_value_section_data.GetObjectNames())
	{
		if (!key_frame_name.contains("KeyFrame"))
		{
			continue;
		}

		AnimationKeyFrame key_frame{};

		JsonObject key_frame_data = animation_value_section_data.GetSubJsonObject("KeyFrame " + std::to_string(i++));
		key_frame_data.LoadData(key_frame.timestamp, "Timestamp");
		key_frame_data.LoadData(*((int*)&key_frame.value_interpolation), "Value_Interpolation");

		if (animation_section.value_type == AnimationValueType::Float)
		{
			key_frame.value_data_id = (uint32_t)storage.animation_value_float_storage.size();
			storage.animation_value_float_storage.push_back({});
			key_frame_data.LoadData(storage.animation_value_float_storage[key_frame.value_data_id], "Value_Float");
		}
		else if (animation_section.value_type == AnimationValueType::Bool)
		{
			key_frame.value_data_id = (uint32_t)storage.animation_value_bool_storage.size();
			storage.animation_value_bool_storage.push_back({});
			bool value;
			key_frame_data.LoadData(value, "Value_Bool");
			storage.animation_value_bool_storage[key_frame.value_data_id] = value;
		}
		else if (animation_section.value_type == AnimationValueType::Int)
		{
			key_frame.value_data_id = (uint32_t)storage.animation_value_int_storage.size();
			storage.animation_value_int_storage.push_back({});
			key_frame_data.LoadData(storage.animation_value_int_storage[key_frame.value_data_id], "Value_Int");
		}
		else if (animation_section.value_type == AnimationValueType::Vector2)
		{
			key_frame.value_data_id = (uint32_t)storage.animation_value_vector2_storage.size();
			storage.animation_value_vector2_storage.push_back({});
			key_frame_data.LoadData(storage.animation_value_vector2_storage[key_frame.value_data_id], "Value_Vector2");
		}
		else if (animation_section.value_type == AnimationValueType::Vector3)
		{
			key_frame.value_data_id = (uint32_t)storage.animation_value_vector3_storage.size();
			storage.animation_value_vector3_storage.push_back({});
			key_frame_data.LoadData(storage.animation_value_vector3_storage[key_frame.value_data_id], "Value_Vector3");
		}

		animation_section.animation_key_frames.push_back(key_frame);
	}
}

std::vector<AnimationValueSection> AnimationManager::LoadAnimatableSprite(JsonObject& entity_data, EntityManager* entity_manager, ValueStorage& storage)
{
	std::vector<AnimationValueSection> return_animation_value_sections;
	JsonObject animation_data = entity_data.GetSubJsonObject("Animatable");
	for (const std::string& object_name : animation_data.GetObjectNames())
	{
		JsonObject animation_value_section_data = animation_data.GetSubJsonObject(object_name);

		std::string fixed_object_name = object_name.substr(0, object_name.length() - sizeof("-AnimationValueSection") + 1);
		std::string component_name = fixed_object_name.substr(0, fixed_object_name.find(":"));
		std::string value_name = fixed_object_name.substr(fixed_object_name.find(":") + 1, fixed_object_name.size());

		AnimationValueSection animation_section;
		animation_value_section_data.LoadData(*((int*)&animation_section.value_type), "ValueType");
		animation_section.value_setter_storage_id = AnimationManager::Get()->GetAnimationValueSetterStorageIndex(component_name, value_name);

		LoadAnimationValueSection(animation_value_section_data, animation_section, storage);

		return_animation_value_sections.push_back(animation_section);
	}

	return return_animation_value_sections;
}

std::vector<AnimationValueSection> AnimationManager::LoadAnimationDataAndGetAnimationValueSections(EntityManager* entity_manager, Entity entity, JsonObject& load_animation, ValueStorage& storage)
{
	{
		JsonObject sprite_data = load_animation.GetSubJsonObject("SpriteData");
		SpriteComponent& sprite = entity_manager->GetComponent<SpriteComponent>(entity);

		std::string sprite_texture_path;
		sprite_data.LoadData(sprite_texture_path, "Texture_Path");

		if (sprite_texture_path != "")
		{
			const TextureHandle texture_handle = RenderCore::Get()->LoadTexture(sprite_texture_path, SceneManager::GetActiveSceneIndex());
			SpriteComponentInterface::LoadTextureToSprite(SceneManager::GetActiveSceneIndex(), entity, texture_handle);

			sprite_data.SetData(texture_handle, "texture_handle");
		}

		const TextureHandle saved_texture_handle = sprite.texture_handle;
		SpriteComponentInterface::LoadSpriteComponent(entity, entity_manager, &sprite_data);

		if (sprite_texture_path == "")
		{
			sprite.texture_handle = saved_texture_handle;
		}
	}

	std::vector<AnimationValueSection> return_animation_value_sections;
	{
		JsonObject animatable_sprite_data = load_animation.GetSubJsonObject("AnimatableSpriteData");
		AnimatableSpriteComponentInterface::LoadAnimatableSpriteComponent(entity, entity_manager, &animatable_sprite_data);

		JsonObject animation_data = load_animation.GetSubJsonObject("Animatable");
		for (const std::string& object_name : animation_data.GetObjectNames())
		{
			JsonObject animation_value_section_data = animation_data.GetSubJsonObject(object_name);

			std::string fixed_object_name = object_name.substr(0, object_name.length() - sizeof("-AnimationValueSection") + 1);
			std::string component_name = fixed_object_name.substr(0, fixed_object_name.find(":"));
			std::string value_name = fixed_object_name.substr(fixed_object_name.find(":") + 1, fixed_object_name.size());

			AnimationValueSection animation_section;
			animation_value_section_data.LoadData(*((int*)&animation_section.value_type), "ValueType");
			animation_section.value_setter_storage_id = AnimationManager::Get()->GetAnimationValueSetterStorageIndex(component_name, value_name);

			LoadAnimationValueSection(animation_value_section_data, animation_section, storage);

			return_animation_value_sections.push_back(animation_section);
		}
	}

	return return_animation_value_sections;
}

void AnimationManager::SetStepAnimationKeyData(const AnimationValueType value_type, const AnimationValueSetterStorage& value_setter_storage, const ValueStorage& value_storage, const AnimationKeyFrame& key_frame, const SceneIndex scene_index, const Entity entity)
{
	if (value_type == AnimationValueType::Float)
	{
		const float current_value = value_storage.animation_value_float_storage[key_frame.value_data_id];
		((AnimationValueSetter<float>)value_setter_storage.value_setter)(entity, scene_index, current_value);
	}
	else if (value_type == AnimationValueType::Vector2)
	{
		const Vector2& current_value = value_storage.animation_value_vector2_storage[key_frame.value_data_id];
		((AnimationValueSetter<Vector2>)value_setter_storage.value_setter)(entity, scene_index, current_value);
	}
	else if (value_type == AnimationValueType::Vector3)
	{
		const Vector3& current_value = value_storage.animation_value_vector3_storage[key_frame.value_data_id];
		((AnimationValueSetter<Vector3>)value_setter_storage.value_setter)(entity, scene_index, current_value);
	}
}

void AnimationManager::SetAnimationKeyData(AnimationKeyFrameId current_key_frame, const AnimationValueSection& animation_value_section, const ValueStorage& value_storage, const SceneIndex scene_index, const Entity entity, const float current_time)
{
	const AnimationValueSetterStorage& value_setter_storage = AnimationManager::Get()->GetAnimationValueSetterStorage(animation_value_section.value_setter_storage_id);

	const AnimationKeyFrame& key_frame = animation_value_section.animation_key_frames[current_key_frame];

	const AnimationKeyFrameId next_key_frame_index = current_key_frame + 1;
	const bool has_next_key_frame = next_key_frame_index < animation_value_section.animation_key_frames.size();

	if (!has_next_key_frame)
	{
		SetStepAnimationKeyData(animation_value_section.value_type, value_setter_storage, value_storage, key_frame, scene_index, entity);
		return;
	}

	const AnimationKeyFrame& next_key_frame = animation_value_section.animation_key_frames[next_key_frame_index];

	if (next_key_frame.value_interpolation == AnimationValueInterpolation::Linear)
	{
		const float key_frame_lerp = (current_time - key_frame.timestamp) / (next_key_frame.timestamp - key_frame.timestamp);

		if (animation_value_section.value_type == AnimationValueType::Float)
		{
			const float current_value = value_storage.animation_value_float_storage[key_frame.value_data_id];
			const float next_value = value_storage.animation_value_float_storage[next_key_frame.value_data_id];
			const float value_diff = next_value - current_value;
			const float new_value = current_value + value_diff * key_frame_lerp;

			((AnimationValueSetter<float>)value_setter_storage.value_setter)(entity, scene_index, new_value);
		}
		else if (animation_value_section.value_type == AnimationValueType::Vector2)
		{
			const Vector2& current_value = value_storage.animation_value_vector2_storage[key_frame.value_data_id];
			const Vector2& next_value = value_storage.animation_value_vector2_storage[next_key_frame.value_data_id];
			const Vector2 value_diff = next_value - current_value;
			const Vector2 new_value = current_value + value_diff * key_frame_lerp;

			((AnimationValueSetter<Vector2>)value_setter_storage.value_setter)(entity, scene_index, new_value);
		}
		else if (animation_value_section.value_type == AnimationValueType::Vector3)
		{
			const Vector3& current_value = value_storage.animation_value_vector3_storage[key_frame.value_data_id];
			const Vector3& next_value = value_storage.animation_value_vector3_storage[next_key_frame.value_data_id];
			const Vector3 value_diff = next_value - current_value;
			const Vector3 new_value = current_value + value_diff * key_frame_lerp;

			((AnimationValueSetter<Vector3>)value_setter_storage.value_setter)(entity, scene_index, new_value);
		}

		return;
	}

	SetStepAnimationKeyData(animation_value_section.value_type, value_setter_storage, value_storage, key_frame, scene_index, entity);
}
