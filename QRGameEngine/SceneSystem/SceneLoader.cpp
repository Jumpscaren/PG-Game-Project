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

SceneLoader* SceneLoader::s_scene_loader = nullptr;

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
			save_file.Write(block_it->second.prefab_data.prefab_index);

			std::vector<std::string> component_name_list = entity_manager->GetComponentNameList(block_it->second.block_entity);

			save_file.Write((uint32_t)component_name_list.size());

			for (int i = 0; i < component_name_list.size(); ++i)
			{
				uint32_t component_name_size = (uint32_t)component_name_list[i].size();
				save_file.Write(component_name_size);
				save_file.Write((void*)component_name_list[i].c_str(), component_name_size);

				auto override_method = m_override_methods.find(component_name_list[i]);
				//Check if there is an override method for the components
				if (override_method == m_override_methods.end())
				{
					ComponentData data = entity_manager->GetComponentData(block_it->second.block_entity, component_name_list[i]);
					save_file.Write(data.component_data, data.component_size);
				}
				else
				{
					override_method->second.save_override_method(block_it->second.block_entity, entity_manager, &save_file);
				}
			}
		}
	}

	save_file.Close();
}

std::unordered_map<uint64_t, std::unordered_map<uint32_t, BlockData>> SceneLoader::LoadScene(std::string scene_name)
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

	m_texture_paths.clear();

	for (uint32_t i = 0; i < number_texture_paths; ++i)
	{
		TextureHandle texture_handle = save_file.Read<TextureHandle>();
		uint32_t text_size = save_file.Read<uint32_t>();
		std::string text;
		text.resize(text_size);
		save_file.Read((void*)text.c_str(), text_size);

		m_texture_paths.insert({ texture_handle, text });
	}

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
			new_block_data.prefab_data.prefab_index = save_file.Read<uint32_t>();
			layer_block_map.insert({ new_block_data.prefab_data.z_index, new_block_data });

			uint32_t component_list_size = save_file.Read<uint32_t>();

			for (uint32_t i = 0; i < component_list_size; ++i)
			{
				uint32_t component_name_size = save_file.Read<uint32_t>();
				std::string component_name;
				component_name.resize(component_name_size);
				save_file.Read((void*)component_name.c_str(), component_name_size);

				auto override_method = m_override_methods.find(component_name);
				//Check if there is an override method for the components
				if (override_method == m_override_methods.end())
				{
					uint32_t component_size = entity_manager->GetComponentSize(component_name);
					char* component_data = (char*)malloc(component_size);
					save_file.Read((void*)component_data, component_size);
					entity_manager->SetComponentData(new_block_data.block_entity, component_name, component_data);
					free(component_data);
				}
				else
				{
					override_method->second.load_override_method(new_block_data.block_entity, entity_manager, &save_file);
				}

				//So that the position is set before instancing the prefab, one case is physics will have the wrong position otherwise
				if (i == 0)
				{
					assert(component_name == "TransformComponent");
					InstancePrefab(game_object, new_block_data.prefab_data.prefab_index);
					//entity_manager->GetComponent<TransformComponent>(new_block).SetPositionZ((float)new_block_data.prefab_data.z_index);
				}
			}
		}

		blocks.insert({ unique_number , layer_block_map });
	}

	save_file.Close();

	return blocks;
}

void SceneLoader::InstancePrefab(const CSMonoObject& game_object, uint32_t prefab_instance_id)
{
	auto mono_core = CSMonoCore::Get();
	mono_core->CallStaticMethod(m_instance_prefab_method, game_object, prefab_instance_id);
}

std::string SceneLoader::GetTexturePath(TextureHandle texture_handle)
{
	return m_texture_paths.find(texture_handle)->second;
}

SceneLoader* SceneLoader::Get()
{
	return s_scene_loader;
}
