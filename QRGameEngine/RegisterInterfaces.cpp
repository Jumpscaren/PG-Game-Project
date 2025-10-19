#include "pch.h"
#include "RegisterInterfaces.h"
#include "Input/Keyboard.h"
#include "Input/Input.h"
#include "Input/Mouse.h"
#include "Components/CameraComponent.h"
#include "Scripting/Objects/SceneInterface.h"
#include "Components/BoxColliderComponent.h"
#include "Components/DynamicBodyComponent.h"
#include "Components/StaticBodyComponent.h"
#include "Components/CircleColliderComponent.h"
#include "Scripting/Objects/Vector2Interface.h"
#include "Components/EntityDataComponent.h"
#include "Scripting/Objects/TimeInterface.h"
#include "Components/AnimatableSpriteComponent.h"
#include "Components/ParentComponent.h"
#include "Scripting/Objects/RenderInterface.h"
#include "Components/TransformComponent.h"
#include "Components/SpriteComponent.h"
#include "Scripting/Objects/GameObjectInterface.h"
#include "Components/ComponentInterface.h"
#include "Components/ScriptComponent.h"
#include "Animation/AnimationManager.h"
#include "Components/PathFindingWorldComponent.h"
#include "Components/PathFindingActorComponent.h"
#include "Components/PureStaticBodyComponent.h"
#include "Scripting/Objects/ListSetInterface.h"
#include "Components/PolygonColliderComponent.h"
#include "Components/KinematicBodyComponent.h"
#include "Scripting/Objects/PhysicsInterface.h"
#include "Components/TileComponent.h"
#include "Scripting/Objects/TextureInterface.h"
#include "SceneSystem/SceneLoader.h"
#include "Physics/PhysicsCore.h"
#include "Scripting/ScriptingManager.h"
#include "Renderer/RenderCore.h"

void RegisterInterfaces::Register(CSMonoCore* const mono_core)
{
	const auto scene_manager_handle = mono_core->RegisterMonoClass("ScriptProject.Engine", "SceneManager");
	mono_core->HookAndRegisterMonoMethodType<SceneManager::GetActiveSceneIndex>(scene_manager_handle, "GetActiveSceneIndex", SceneManager::GetActiveSceneIndex);
	const auto entity_manager_handle = mono_core->RegisterMonoClass("ScriptProject.Engine", "EntityManager");
	mono_core->HookAndRegisterMonoMethodType<EntityManager::CreateEntity>(entity_manager_handle, "CreateEntity", EntityManager::CreateEntity);
	mono_core->HookAndRegisterMonoMethodType<EntityManager::DeleteEntity>(entity_manager_handle, "DeleteEntity", EntityManager::DeleteEntity);

	// Register Defered Calls
	// Needs to be according to call order
	const DeferedMethodIndex remove_physic_object_index = SceneLoader::Get()->GetDeferedCalls()->Register<&PhysicsCore::RemovePhysicObject>(PhysicsCore::Get());
	const DeferedMethodIndex add_physic_object_index = SceneLoader::Get()->GetDeferedCalls()->Register<&PhysicsCore::AddPhysicObject>(PhysicsCore::Get());

	const DeferedMethodIndex remove_box_collider_index = SceneLoader::Get()->GetDeferedCalls()->Register<&PhysicsCore::RemoveBoxCollider>(PhysicsCore::Get());
	const DeferedMethodIndex add_box_collider_index = SceneLoader::Get()->GetDeferedCalls()->Register<&PhysicsCore::AddBoxCollider>(PhysicsCore::Get());

	const DeferedMethodIndex remove_circle_collider_index = SceneLoader::Get()->GetDeferedCalls()->Register<&PhysicsCore::RemoveCircleCollider>(PhysicsCore::Get());
	const DeferedMethodIndex add_circle_collider_index = SceneLoader::Get()->GetDeferedCalls()->Register<&PhysicsCore::AddCircleCollider>(PhysicsCore::Get());

	const DeferedMethodIndex remove_polygon_collider_index = SceneLoader::Get()->GetDeferedCalls()->Register<&PhysicsCore::RemovePolygonCollider>(PhysicsCore::Get());
	const DeferedMethodIndex add_polygon_collider_index = SceneLoader::Get()->GetDeferedCalls()->Register<&PhysicsCore::AddPolygonCollider>(PhysicsCore::Get());

	const DeferedMethodIndex remove_script_index = SceneLoader::Get()->GetDeferedCalls()->Register<&ScriptingManager::RemoveScript>(ScriptingManager::Get());
	const DeferedMethodIndex add_script_index = SceneLoader::Get()->GetDeferedCalls()->Register<&ScriptingManager::AddScript>(ScriptingManager::Get());

	const DeferedMethodIndex load_and_set_texture_index = SceneLoader::Get()->GetDeferedCalls()->Register<&RenderCore::LoadAndSetTexture>(RenderCore::Get());
	const DeferedMethodIndex load_texture_index = SceneLoader::Get()->GetDeferedCalls()->RegisterStatic<RenderInterface::LoadAndSetTexture>();

	const DeferedMethodIndex load_texture_object_sprite_index = SceneLoader::Get()->GetDeferedCalls()->RegisterStatic<SpriteComponentInterface::LoadTextureObjectToSprite>();

	TransformComponentInterface::RegisterInterface(mono_core);
	SpriteComponentInterface::RegisterInterface(mono_core, load_and_set_texture_index, load_texture_object_sprite_index);
	GameObjectInterface::RegisterInterface(mono_core);
	SceneInterface::RegisterInterface(mono_core);
	ComponentInterface::RegisterInterface(mono_core);
	RenderInterface::RegisterInterface(mono_core, load_texture_index);
	ScriptComponentInterface::RegisterInterface(mono_core, add_script_index, remove_script_index);
	InputInterface::RegisterInterface(mono_core);
	CameraComponentInterface::RegisterInterface(mono_core);
	DynamicBodyComponentInterface::RegisterInterface(mono_core, add_physic_object_index, remove_physic_object_index);
	BoxColliderComponentInterface::RegisterInterface(mono_core, add_physic_object_index, add_box_collider_index, remove_box_collider_index);
	CircleColliderComponentInterface::RegisterInterface(mono_core, add_physic_object_index, add_circle_collider_index, remove_circle_collider_index);
	StaticBodyComponentInterface::RegisterInterface(mono_core, add_physic_object_index, remove_physic_object_index);
	Vector2Interface::RegisterInterface(mono_core);
	EntityDataComponentInterface::RegisterInterface(mono_core);
	AnimatableSpriteComponentInterface::RegisterInterface(mono_core);
	TimeInterface::RegisterInterface(mono_core);
	AnimationManager::RegisterInterface(mono_core);
	PathFindingWorldComponentInterface::RegisterInterface(mono_core);
	PathFindingActorComponentInterface::RegisterInterface(mono_core);
	ListSetInterface::RegisterInterface(mono_core);
	PureStaticBodyComponentInterface::RegisterInterface(mono_core, add_physic_object_index, remove_physic_object_index);
	PolygonColliderComponentInterface::RegisterInterface(mono_core, add_physic_object_index, add_polygon_collider_index, remove_polygon_collider_index);
	KinematicBodyComponentInterface::RegisterInterface(mono_core, add_physic_object_index, remove_physic_object_index);
	PhysicsInterface::RegisterInterface(mono_core);
	TileComponentInterface::RegisterInterface(mono_core);
	ParentComponentInterface::RegisterInterface(mono_core);
	TextureInterface::RegisterInterface(mono_core);
}
