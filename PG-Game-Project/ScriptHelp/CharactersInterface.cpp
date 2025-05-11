#include "CharactersInterface.h"
#include "ECS/EntityManager.h"
#include "SceneSystem/SceneManager.h"
#include "Scripting/Objects/GameObjectInterface.h"
#include "Scripting/Objects/ListSetInterface.h"
#include "Components/DynamicBodyComponent.h"
#include "Components/ScriptComponent.h"
#include "SceneSystem/GlobalScene.h"
#include "Scripting/CSMonoCore.h"

void CharactersInterface::RegisterInterface()
{
	CSMonoCore* mono_core = CSMonoCore::Get();

	const auto characters_interface_class = mono_core->RegisterMonoClass("ScriptProject.UserDefined", "CharactersInterface");

	mono_core->HookAndRegisterMonoMethodType<CharactersInterface::GetInteractiveCharacters>(characters_interface_class, "GetInteractiveCharacters", CharactersInterface::GetInteractiveCharacters);
}

void CharactersInterface::GetInteractiveCharacters(const CSMonoObject& list)
{
    const MonoClassHandle class_handle = CSMonoCore::Get()->RegisterMonoClass("ScriptProject.Scripts", "InteractiveCharacterBehaviour");

	SceneIndex scene_index = SceneManager::GetActiveSceneIndex();
	const auto add_interactive_characters = [&](const Entity entity, const DynamicBodyComponent& entity_data, const ScriptComponent& script)
		{
			if (!CSMonoCore::Get()->IsOfClass(script.script_object, class_handle))
			{
				return;
			}

			const auto game_object = GameObjectInterface::NewGameObjectWithExistingEntity(entity, scene_index);
			ListSetInterface::AddGameObject(list, game_object);
		};

    SceneManager::GetSceneManager()->GetEntityManager(SceneManager::GetSceneManager()->GetActiveSceneIndex())->System<DynamicBodyComponent, ScriptComponent>(add_interactive_characters);

	scene_index = GlobalScene::Get()->GetSceneIndex();
    SceneManager::GetSceneManager()->GetEntityManager(GlobalScene::Get()->GetSceneIndex())->System<DynamicBodyComponent, ScriptComponent>(add_interactive_characters);
}
