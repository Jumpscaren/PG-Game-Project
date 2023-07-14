#include "pch.h"
#include "GameObjectInterface.h"
#include "Scripting/CSMonoCore.h"

MonoFieldHandle GameObjectInterface::get_entity_id_field;
MonoClassHandle GameObjectInterface::game_object_class;
MonoMethodHandle GameObjectInterface::get_scene_index_method;
MonoMethodHandle GameObjectInterface::create_game_object_method;

void GameObjectInterface::RegisterInterface(CSMonoCore* mono_core)
{
    game_object_class = mono_core->RegisterMonoClass("ScriptProject.Engine", "GameObject");
    get_scene_index_method = mono_core->RegisterMonoMethod(game_object_class, "GetSceneIndex");
    get_entity_id_field = mono_core->RegisterField(game_object_class, "entity_id");
    create_game_object_method = mono_core->RegisterMonoMethod(game_object_class, "CreateGameObject");
}

Entity GameObjectInterface::GetEntityID(const CSMonoObject& game_object)
{
    Entity entity_id;
    CSMonoCore::Get()->GetValue(entity_id, game_object, get_entity_id_field);
    return entity_id;
}

SceneIndex GameObjectInterface::GetSceneIndex(const CSMonoObject& game_object)
{
    SceneIndex scene_index;
    CSMonoCore::Get()->CallMethod(scene_index, get_scene_index_method, game_object);
    return scene_index;
}

CSMonoObject GameObjectInterface::CreateGameObject()
{
    CSMonoObject game_object;
    CSMonoCore::Get()->CallStaticMethod(game_object, create_game_object_method);

    return game_object;
}
