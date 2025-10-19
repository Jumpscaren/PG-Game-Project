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
#include "Time/Timer.h"
#include "Common/DeferMethodCalls.h"

SceneLoader* SceneLoader::s_scene_loader = nullptr;

Entity SceneLoader::LoadComponents(OutputFile* save_file, EntityManager* entity_manager, Entity entity)
{
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
			LoadComponent(&component, component_names[i], entity, entity_manager);
		}
	}

	Entity saved_entity = NULL_ENTITY;
	if (components_json.ObjectExist("Entity"))
	{
		components_json.LoadData(saved_entity, "Entity");
	}

	return saved_entity;
}

SceneLoader::SceneLoader()
{
	s_scene_loader = this;

	auto mono_core = CSMonoCore::Get();
	m_prefab_instancer_class = mono_core->RegisterMonoClass("ScriptProject.Engine", "PrefabSystem");
	m_instance_prefab_method = mono_core->RegisterMonoMethod(m_prefab_instancer_class, "InstanceUserPrefab");
}

SceneLoader::~SceneLoader()
{
	if (!m_load_scene_user_controlled)
	{
		return;
	}

	m_threaded_scene_loader_finished = true;
	/* FIX THIS KILL LOAD THREAD/*/
	//HandleSceneLoadingPostUser();
}

void SceneLoader::SaveScene(qr::unordered_map<uint64_t, qr::unordered_map<uint32_t, BlockData>>& blocks, std::string scene_name)
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

			JsonObject transform_json_object;
			SaveComponent(&transform_json_object, "TransformComponent", block_it->second.block_entity, entity_manager);
			std::string transform_json_string = transform_json_object.GetJsonString();
			save_file.Write((uint32_t)transform_json_string.size());
			save_file.Write((void*)transform_json_string.c_str(), (uint32_t)transform_json_string.size());

			std::vector<std::string> component_name_list = entity_manager->GetComponentNameList(block_it->second.block_entity);

			save_file.Write((uint32_t)component_name_list.size());
			JsonObject json_object;
			json_object.SetData(block_it->second.block_entity, "Entity");

			for (int i = 0; i < component_name_list.size(); ++i)
			{
				uint32_t component_name_size = (uint32_t)component_name_list[i].size();
				save_file.Write(component_name_size);
				save_file.Write((void*)component_name_list[i].c_str(), component_name_size);

				JsonObject component_json_object = json_object.CreateSubJsonObject(component_name_list[i]);

				SaveComponent(&component_json_object, component_name_list[i], block_it->second.block_entity, entity_manager);
			}
			
			std::string json_string = json_object.GetJsonString();
			save_file.Write((uint32_t)json_string.size());
			save_file.Write((void*)json_string.c_str(), (uint32_t)json_string.size());
		}
	}

	save_file.Close();
}

qr::unordered_map<uint64_t, qr::unordered_map<uint32_t, BlockData>> SceneLoader::LoadSceneEditor(std::string scene_name)
{
	qr::unordered_map<uint64_t, qr::unordered_map<uint32_t, BlockData>> blocks;

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

	qr::unordered_map<Entity, Entity> new_entity_to_saved_entity_map;
	std::string prefab_name;
	for (uint32_t i = 0; i < numberofblocks; ++i)
	{
		uint64_t unique_number = save_file.Read<uint64_t>();
		uint64_t block_layers = save_file.Read<uint64_t>();

		qr::unordered_map<uint32_t, BlockData> layer_block_map;

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

			LoadTransformComponent(&save_file, new_block, entity_manager);

			InstancePrefab(game_object, prefab_name);

			const Entity saved_entity = LoadComponents(&save_file, entity_manager, new_block);

			new_entity_to_saved_entity_map.insert({ new_block, saved_entity });
		}

		blocks.insert({ unique_number , layer_block_map });
	}

	save_file.Close();

	return blocks;
}

void SceneLoader::LoadScene(std::string scene_name, SceneIndex load_scene, bool threaded)
{
	Timer timer;
	if (threaded)
	{
		m_mono_thread_handle = CSMonoCore::Get()->HookThread();
	}

	std::string scene_name_file;
	//So goddamn stupid!!!
	scene_name_file.insert(0, scene_name.c_str());
	scene_name_file += ".scene";
	OutputFile save_file = Output::LoadCompressedOutputFile(scene_name_file);

	assert(save_file.FileExists());

	SceneManager* scene_manager = SceneManager::GetSceneManager();
	EntityManager* entity_manager = scene_manager->GetEntityManager(load_scene);

	uint32_t numberofblocks = save_file.Read<uint32_t>();
	std::string prefab_name;
	for (uint32_t i = 0; i < numberofblocks; ++i)
	{
		uint64_t unique_number = save_file.Read<uint64_t>();
		uint64_t block_layers = save_file.Read<uint64_t>();

		for (uint64_t layers = 0; layers < block_layers; ++layers)
		{
			save_file.Read<uint32_t>();
			uint32_t text_size = save_file.Read<uint32_t>();
			prefab_name.resize(text_size);
			save_file.Read((void*)prefab_name.c_str(), text_size);

			Entity new_entity = entity_manager->NewEntity();
			SpriteComponent& sprite = entity_manager->AddComponent<SpriteComponent>(new_entity);

			CSMonoObject game_object = GameObjectInterface::NewGameObjectWithExistingEntity(new_entity, load_scene);
			LoadTransformComponent(&save_file, new_entity, entity_manager);
			InstancePrefab(game_object, prefab_name);

			LoadComponents(&save_file, entity_manager, new_entity);
		}
	}

	save_file.Close();

	if (threaded)
	{
		CSMonoCore::Get()->UnhookThread(m_mono_thread_handle);
	}

	const auto time = timer.StopTimer();
	std::cout << "Loaded scene " << scene_name << " in " << time / double(Timer::TimeTypes::Seconds) << "s" << std::endl;

	m_threaded_scene_loader_finished = true;
}

void SceneLoader::LoadSceneThreaded(std::string scene_name, SceneIndex load_scene)
{
	assert(m_load_scene_thread == nullptr);
	m_threaded_scene_loader_finished = false;
	//m_physic_update_thread = new std::thread(&PhysicsCore::ThreadUpdatePhysic, this);
	//LoadScene(scene_name, load_scene);
	//m_load_scene_mutex.lock();
	m_load_scene_index = load_scene;
	m_load_scene_thread = new std::thread(&SceneLoader::LoadScene, this, scene_name, load_scene, true);
}

void SceneLoader::SaveComponent(JsonObject* component_json_object, const std::string& component_name, Entity entity, EntityManager* entity_manager)
{
	auto override_method = m_override_methods.find(component_name);
	//Check if there is an override method for the components
	if (override_method == m_override_methods.end())
	{
		ComponentData data = entity_manager->GetComponentData(entity, component_name);
		component_json_object->SetData(data.component_data, data.component_size, "b_data");
		component_json_object->SetData(data.component_size, "b_size");
	}
	else
	{
		override_method->second.save_override_method(entity, entity_manager, component_json_object);
	}
}

void SceneLoader::LoadTransformComponent(OutputFile* save_file, Entity entity, EntityManager* entity_manager)
{
	std::string transform_json_text;
	uint32_t json_text_size = save_file->Read<uint32_t>();
	transform_json_text.resize(json_text_size);
	save_file->Read((void*)transform_json_text.c_str(), json_text_size);
	JsonObject components_json(transform_json_text);
	LoadComponent(&components_json, "TransformComponent", entity, entity_manager);
}

void SceneLoader::LoadComponent(JsonObject* component_json_object, const std::string& component_name, Entity entity, EntityManager* entity_manager)
{
	const uint32_t component_data_max_size = 2000;
	char component_data[component_data_max_size];

	const auto override_method = m_override_methods.find(component_name);
	if (override_method == m_override_methods.end())
	{
		uint32_t component_size;
		component_json_object->LoadData(component_size, "b_size");
		assert(component_size <= component_data_max_size);
		component_json_object->LoadData(component_data, component_size, "b_data");
		entity_manager->SetComponentData(entity, component_name, component_data);
	}
	else
	{
		override_method->second.load_override_method(entity, entity_manager, component_json_object);
	}
}

void SceneLoader::InstancePrefab(const CSMonoObject& game_object, std::string prefab_name)
{
	auto mono_core = CSMonoCore::Get();
	mono_core->CallStaticMethod(m_instance_prefab_method, game_object, prefab_name);
}

void SceneLoader::HandleDeferedCalls()
{
	if (!m_load_scene_thread)
	{
		return;
	}

	if (m_deferred_calls.IsFull())
	{
		m_deferred_calls.ClearBuffers();
		return;
	}

	if (m_threaded_scene_loader_finished)
	{
		m_load_scene_thread->join();
		delete m_load_scene_thread;
		m_load_scene_thread = nullptr;
		m_threaded_scene_loader_finished = false;

		m_deferred_calls.ClearBuffers();

		SceneManager::GetSceneManager()->GetScene(m_load_scene_index)->SetSceneAsLoaded();
		m_load_scene_index = NULL_SCENE_INDEX;
	}
}

bool SceneLoader::FinishedLoadingScene()
{
	return m_load_scene_index == NULL_SCENE_INDEX;
}

SceneIndex SceneLoader::GetLoadingScene() const
{
	return m_load_scene_index;
}

bool SceneLoader::DeferMethodCall() const
{
	return m_load_scene_thread && m_load_scene_thread->get_id() == std::this_thread::get_id();
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
