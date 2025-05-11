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

void RegisterInterfaces::Register(CSMonoCore* const mono_core)
{
	const auto scene_manager_handle = mono_core->RegisterMonoClass("ScriptProject.Engine", "SceneManager");
	mono_core->HookAndRegisterMonoMethodType<SceneManager::GetActiveSceneIndex>(scene_manager_handle, "GetActiveSceneIndex", SceneManager::GetActiveSceneIndex);
	const auto entity_manager_handle = mono_core->RegisterMonoClass("ScriptProject.Engine", "EntityManager");
	mono_core->HookAndRegisterMonoMethodType<EntityManager::CreateEntity>(entity_manager_handle, "CreateEntity", EntityManager::CreateEntity);
	mono_core->HookAndRegisterMonoMethodType<EntityManager::DeleteEntity>(entity_manager_handle, "DeleteEntity", EntityManager::DeleteEntity);

	TransformComponentInterface::RegisterInterface(mono_core);
	SpriteComponentInterface::RegisterInterface(mono_core);
	GameObjectInterface::RegisterInterface(mono_core);
	SceneInterface::RegisterInterface(mono_core);
	ComponentInterface::RegisterInterface(mono_core);
	RenderInterface::RegisterInterface(mono_core);
	ScriptComponentInterface::RegisterInterface(mono_core);
	InputInterface::RegisterInterface(mono_core);
	CameraComponentInterface::RegisterInterface(mono_core);
	DynamicBodyComponentInterface::RegisterInterface(mono_core);
	BoxColliderComponentInterface::RegisterInterface(mono_core);
	CircleColliderComponentInterface::RegisterInterface(mono_core);
	StaticBodyComponentInterface::RegisterInterface(mono_core);
	Vector2Interface::RegisterInterface(mono_core);
	EntityDataComponentInterface::RegisterInterface(mono_core);
	AnimatableSpriteComponentInterface::RegisterInterface(mono_core);
	TimeInterface::RegisterInterface(mono_core);
	AnimationManager::RegisterInterface(mono_core);
	PathFindingWorldComponentInterface::RegisterInterface(mono_core);
	PathFindingActorComponentInterface::RegisterInterface(mono_core);
	ListSetInterface::RegisterInterface(mono_core);
	PureStaticBodyComponentInterface::RegisterInterface(mono_core);
	PolygonColliderComponentInterface::RegisterInterface(mono_core);
	KinematicBodyComponentInterface::RegisterInterface(mono_core);
	PhysicsInterface::RegisterInterface(mono_core);
	TileComponentInterface::RegisterInterface(mono_core);
}
