#pragma once
#include "ECS/EntityDefinition.h"
#include <functional>
#include "Editor/DrawScene.h"
#include "Scripting/CSMonoObject.h"
#include "SceneLoaderTypes.h"
#include "SceneDefines.h"
#include <thread>
#include <mutex>
#include "Helpers/SceneLoaderDeferCalls.h"

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
	qr::unordered_map<std::string, OverrideSaveLoadMethods> m_override_methods;

	std::thread* m_load_scene_thread = nullptr;
	bool m_load_scene_user_controlled = false;
	std::atomic<bool> m_threaded_scene_loader_finished = false;
	SceneIndex m_load_scene_index = NULL_SCENE_INDEX;
	MonoThreadHandle m_mono_thread_handle;

	SceneLoaderDeferCalls m_deferred_calls;

private:
	Entity LoadComponents(OutputFile* save_file, EntityManager* entity_manager, Entity enitity);
	void LoadScene(std::string scene_name, SceneIndex load_scene, bool threaded);
	void LoadSceneThreaded(std::string scene_name, SceneIndex load_scene);

	void SaveComponent(JsonObject* component_json_object, const std::string& component_name, Entity entity, EntityManager* entity_manager);
	void LoadTransformComponent(OutputFile* save_file, Entity entity, EntityManager* entity_manager);
	void LoadComponent(JsonObject* component_json_object, const std::string& component_name, Entity entity, EntityManager* entity_manager);

public:
	SceneLoader();
	~SceneLoader();

	void SaveScene(qr::unordered_map<uint64_t, qr::unordered_map<uint32_t, BlockData>>& blocks, std::string scene_name);
	qr::unordered_map<uint64_t, qr::unordered_map<uint32_t, BlockData>> LoadSceneEditor(std::string scene_name);

	static SceneLoader* Get();

	void InstancePrefab(const CSMonoObject& game_object, std::string prefab_name);

	void HandleDeferedCalls();
	SceneLoaderDeferCalls* GetDeferedCalls() { return &m_deferred_calls; }

	bool FinishedLoadingScene();
	SceneIndex GetLoadingScene() const;
	bool DeferMethodCall() const;

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