#include "pch.h"
#include "SceneLoader.h"
#include "SceneManager.h"
#include "IO/Output.h"
#include "EngineComponents.h"
#include "Components/SpriteComponent.h"
#include "Asset/AssetManager.h"
#include "Renderer/RenderCore.h"
#include "Scripting/Objects/GameObjectInterface.h"
#include "Scripting/CSMonoCore.h"
#include "IO/JsonObject.h"

SceneLoader* SceneLoader::s_scene_loader = nullptr;

void SceneLoader::LoadTexturePaths(OutputFile* save_file, uint32_t number_of_texture_paths)
{
	m_texture_paths.clear();

	std::string text;
	for (uint32_t i = 0; i < number_of_texture_paths; ++i)
	{
		TextureHandle texture_handle = save_file->Read<TextureHandle>();
		uint32_t text_size = save_file->Read<uint32_t>();
		text.resize(text_size);
		save_file->Read((void*)text.c_str(), text_size);

		m_texture_paths.insert({ texture_handle, text });
	}
}

void SceneLoader::LoadComponents(OutputFile* save_file, EntityManager* entity_manager, Entity entity)
{
	const uint32_t component_data_max_size = 2000;
	char component_data[component_data_max_size];

	uint32_t component_list_size = save_file->Read<uint32_t>();
	std::vector<std::string> component_names;
	component_names.resize(component_list_size);
	for (uint32_t i = 0; i < component_list_size; ++i)
	{
		uint32_t component_name_size = save_file->Read<uint32_t>();
		component_names[i].resize(component_name_size);
		save_file->Read((void*)component_names[i].c_str(), component_name_size);
	}

	std::string json_text;
	uint32_t json_text_size = save_file->Read<uint32_t>();
	json_text.resize(json_text_size);
	save_file->Read((void*)json_text.c_str(), json_text_size);
	JsonObject components_json(json_text);
	for (uint32_t i = 0; i < component_list_size; ++i)
	{
		if (entity_manager->HasComponentName(entity, component_names[i]) && components_json.ObjectExist(component_names[i]))
		{
			JsonObject component = components_json.GetSubJsonObject(component_names[i]);
			const auto override_method = m_override_methods.find(component_names[i]);
			if (override_method == m_override_methods.end())
			{
				uint32_t component_size;
				component.LoadData(component_size, "b_size");
				assert(component_size <= component_data_max_size);
				component.LoadData(component_data, component_size, "b_data");
				entity_manager->SetComponentData(entity, component_names[i], component_data);
			}
			else
			{
				override_method->second.load_override_method(entity, entity_manager, &component);
			}
		}
	}
}

SceneLoader::SceneLoader()
{
	s_scene_loader = this;

	auto mono_core = CSMonoCore::Get();
	m_prefab_instancer_class = mono_core->RegisterMonoClass("ScriptProject.Engine", "PrefabSystem");
	m_instance_prefab_method = mono_core->RegisterMonoMethod(m_prefab_instancer_class, "InstanceUserPrefab");
}

void SceneLoader::SaveScene(std::unordered_map<uint64_t, std::unordered_map<uint32_t, BlockData>>& blocks, std::string scene_name)
{
	SceneManager* scene_manager = SceneManager::GetSceneManager();
	EntityManager* entity_manager = scene_manager->GetScene(scene_manager->GetActiveSceneIndex())->GetEntityManager();

	std::string scene_name_file;
	//So goddamn stupid!!!
	scene_name_file.insert(0, scene_name.c_str());
	scene_name_file += ".scene";
	OutputFile save_file = Output::CreateCompressedOutputFile(scene_name_file);

	if (!save_file.FileExists())
		return;

	uint32_t numberofblocks = (uint32_t)blocks.size();
	save_file.Write(numberofblocks);

	std::unordered_map<TextureHandle, std::string> texture_paths;

	for (auto it = blocks.begin(); it != blocks.end(); it++)
	{
		for (auto block_it = it->second.begin(); block_it != it->second.end(); block_it++)
		{
			if (!entity_manager->HasComponent<SpriteComponent>(block_it->second.block_entity))
			{
				continue;
			}

			TextureHandle texture_handle = entity_manager->GetComponent<SpriteComponent>(block_it->second.block_entity).texture_handle;

			if (texture_paths.find(texture_handle) == texture_paths.end())
			{
				std::string texture_path = AssetManager::Get()->GetAssetPath(RenderCore::Get()->GetTextureAssetHandle(texture_handle));
				texture_paths.insert({ texture_handle, texture_path });
			}
		}
	}

	save_file.Write((uint32_t)texture_paths.size());
	for (auto it = texture_paths.begin(); it != texture_paths.end(); it++)
	{
		save_file.Write(it->first);
		save_file.Write((uint32_t)it->second.size());
		save_file.Write((void*)it->second.c_str(), (uint32_t)it->second.size());
	}

	for (auto it = blocks.begin(); it != blocks.end(); it++)
	{
		save_file.Write(it->first);
		save_file.Write((uint64_t)it->second.size());

		for (auto block_it = it->second.begin(); block_it != it->second.end(); block_it++)
		{
			save_file.Write(block_it->first);
			const auto& prefab_name = block_it->second.prefab_data.prefab_name;
			save_file.Write((uint32_t)prefab_name.size());
			save_file.Write((void*)prefab_name.c_str(), (uint32_t)prefab_name.size());

			std::vector<std::string> component_name_list = entity_manager->GetComponentNameList(block_it->second.block_entity);

			save_file.Write((uint32_t)component_name_list.size());
			JsonObject json_object;

			for (int i = 0; i < component_name_list.size(); ++i)
			{
				uint32_t component_name_size = (uint32_t)component_name_list[i].size();
				save_file.Write(component_name_size);
				save_file.Write((void*)component_name_list[i].c_str(), component_name_size);

				JsonObject component_json_object = json_object.CreateSubJsonObject(component_name_list[i]);

				auto override_method = m_override_methods.find(component_name_list[i]);
				//Check if there is an override method for the components
				if (override_method == m_override_methods.end())
				{
					ComponentData data = entity_manager->GetComponentData(block_it->second.block_entity, component_name_list[i]);
					component_json_object.SetData(data.component_data, data.component_size, "b_data");
					component_json_object.SetData(data.component_size, "b_size");
				}
				else
				{
					override_method->second.save_override_method(block_it->second.block_entity, entity_manager, &component_json_object);
				}
			}
			
			std::string json_string = json_object.GetJsonString();
			save_file.Write((uint32_t)json_string.size());
			save_file.Write((void*)json_string.c_str(), (uint32_t)json_string.size());
		}
	}

	save_file.Close();
}

std::unordered_map<uint64_t, std::unordered_map<uint32_t, BlockData>> SceneLoader::LoadSceneEditor(std::string scene_name)
{
	std::unordered_map<uint64_t, std::unordered_map<uint32_t, BlockData>> blocks;

	SceneManager* scene_manager = SceneManager::GetSceneManager();
	EntityManager* entity_manager = scene_manager->GetScene(scene_manager->GetActiveSceneIndex())->GetEntityManager();

	std::string scene_name_file;
	//So goddamn stupid!!!
	scene_name_file.insert(0, scene_name.c_str());
	scene_name_file += ".scene";
	OutputFile save_file = Output::LoadCompressedOutputFile(scene_name_file);

	if (!save_file.FileExists())
		return blocks;

	uint32_t numberofblocks = save_file.Read<uint32_t>();

	uint32_t number_texture_paths = save_file.Read<uint32_t>();

	LoadTexturePaths(&save_file, number_texture_paths);

	std::string prefab_name;
	for (uint32_t i = 0; i < numberofblocks; ++i)
	{
		uint64_t unique_number = save_file.Read<uint64_t>();
		uint64_t block_layers = save_file.Read<uint64_t>();

		std::unordered_map<uint32_t, BlockData> layer_block_map;

		for (uint64_t layers = 0; layers < block_layers; ++layers)
		{
			CSMonoObject game_object = GameObjectInterface::CreateGameObject();
			Entity new_block = GameObjectInterface::GetEntityID(game_object);
			SpriteComponent& sprite = entity_manager->AddComponent<SpriteComponent>(new_block);

			BlockData new_block_data;
			new_block_data.block_entity = new_block;
			new_block_data.prefab_data.z_index = save_file.Read<uint32_t>();
			uint32_t text_size = save_file.Read<uint32_t>();
			prefab_name.resize(text_size);
			save_file.Read((void*)prefab_name.c_str(), text_size);
			new_block_data.prefab_data.prefab_name = prefab_name;
			layer_block_map.insert({ new_block_data.prefab_data.z_index, new_block_data });
			InstancePrefab(game_object, prefab_name);

			LoadComponents(&save_file, entity_manager, new_block);
		}

		blocks.insert({ unique_number , layer_block_map });
	}

	save_file.Close();

	m_texture_paths.clear();

	return blocks;
}

void SceneLoader::LoadScene(std::string scene_name, SceneIndex load_scene)
{
	std::string scene_name_file;
	//So goddamn stupid!!!
	scene_name_file.insert(0, scene_name.c_str());
	scene_name_file += ".scene";
	OutputFile save_file = Output::LoadCompressedOutputFile(scene_name_file);

	assert(save_file.FileExists());

	SceneManager* scene_manager = SceneManager::GetSceneManager();
	EntityManager* entity_manager = scene_manager->GetEntityManager(load_scene);

	uint32_t numberofblocks = save_file.Read<uint32_t>();

	uint32_t number_texture_paths = save_file.Read<uint32_t>();

	LoadTexturePaths(&save_file, number_texture_paths);

	std::string prefab_name;
	for (uint32_t i = 0; i < numberofblocks; ++i)
	{
		uint64_t unique_number = save_file.Read<uint64_t>();
		uint64_t block_layers = save_file.Read<uint64_t>();

		for (uint64_t layers = 0; layers < block_layers; ++layers)
		{
			Entity new_entity = entity_manager->NewEntity();
			CSMonoObject game_object = GameObjectInterface::NewGameObjectWithExistingEntity(new_entity, load_scene);
			SpriteComponent& sprite = entity_manager->AddComponent<SpriteComponent>(new_entity);
			save_file.Read<uint32_t>();
			uint32_t text_size = save_file.Read<uint32_t>();
			prefab_name.resize(text_size);
			save_file.Read((void*)prefab_name.c_str(), text_size);
			InstancePrefab(game_object, prefab_name);

			LoadComponents(&save_file, entity_manager, new_entity);
		}
	}

	save_file.Close();

	m_texture_paths.clear();
}

void SceneLoader::InstancePrefab(const CSMonoObject& game_object, std::string prefab_name)
{
	auto mono_core = CSMonoCore::Get();
	mono_core->CallStaticMethod(m_instance_prefab_method, game_object, prefab_name);
}

std::string SceneLoader::GetTexturePath(TextureHandle texture_handle)
{
	return m_texture_paths.find(texture_handle)->second;
}

bool SceneLoader::HasTexturePath(const TextureHandle texture_handle)
{
	return m_texture_paths.contains(texture_handle);
}

std::function<void(Entity, EntityManager*, JsonObject*)>* SceneLoader::GetOverrideSaveComponentMethod(const std::string& component_name)
{
	if (m_override_methods.contains(component_name))
	{
		return &(m_override_methods.find(component_name)->second.save_override_method);
	}
	return nullptr;
}

std::function<void(Entity, EntityManager*, JsonObject*)>* SceneLoader::GetOverrideLoadComponentMethod(const std::string& component_name)
{
	if (m_override_methods.contains(component_name))
	{
		return &(m_override_methods.find(component_name)->second.load_override_method);
	}
	return nullptr;
}

SceneLoader* SceneLoader::Get()
{
	return s_scene_loader;
}
