#pragma once
#include "ECS/EntityDefinition.h"
#include <functional>
#include "Editor/DrawScene.h"
#include "Scripting/CSMonoObject.h"
#include "SceneLoaderTypes.h"
#include "SceneDefines.h"

class EntityManager;
class OutputFile;
class SceneManager;
class JsonObject;

class SceneLoader
{
	friend SceneManager;
private:
	struct OverrideSaveLoadMethods
	{
		std::function<void(Entity, EntityManager*, JsonObject*)> save_override_method;
		std::function<void(Entity, EntityManager*, JsonObject*)> load_override_method;
	};

private:
	MonoMethodHandle m_instance_prefab_method;
	MonoClassHandle m_prefab_instancer_class;
	static SceneLoader* s_scene_loader;
	std::unordered_map<std::string, OverrideSaveLoadMethods> m_override_methods;
	std::unordered_map<TextureHandle, std::string> m_texture_paths;

private:
	void LoadTexturePaths(OutputFile* save_file, uint32_t number_of_texture_paths);
	void LoadComponents(OutputFile* save_file, EntityManager* entity_manager, Entity enitity);
	void LoadScene(std::string scene_name, SceneIndex load_scene);

public:
	SceneLoader();

	void SaveScene(std::unordered_map<uint64_t, std::unordered_map<uint32_t, BlockData>>& blocks, std::string scene_name);
	std::unordered_map<uint64_t, std::unordered_map<uint32_t, BlockData>> LoadSceneEditor(std::string scene_name);

	static SceneLoader* Get();

	void InstancePrefab(const CSMonoObject& game_object, uint32_t prefab_instance_id);

	std::string GetTexturePath(TextureHandle texture_handle);

	template<typename Component>
	void OverrideSaveComponentMethod(std::function<void(Entity, EntityManager*, JsonObject*)> override_save_method, std::function<void(Entity, EntityManager*, JsonObject*)> override_load_method);

	std::function<void(Entity, EntityManager*, JsonObject*)>* GetOverrideSaveComponentMethod(const std::string& component_name);
	std::function<void(Entity, EntityManager*, JsonObject*)>* GetOverrideLoadComponentMethod(const std::string& component_name);
};

template<typename Component>
inline void SceneLoader::OverrideSaveComponentMethod(std::function<void(Entity, EntityManager*, JsonObject*)> override_save_method, std::function<void(Entity, EntityManager*, JsonObject*)> override_load_method)
{
	std::string component_name = EntityManager::GetComponentNameFromComponent<Component>();
	OverrideSaveLoadMethods methods = {};
	methods.save_override_method = override_save_method;
	methods.load_override_method = override_load_method;

	m_override_methods.insert({ component_name, methods });
}