#include "pch.h"
#include "QREntryPoint.h"
#include "Renderer/RenderCore.h"
#include "ECS/EntityManager.h"
#include "SceneSystem/SceneManager.h"
#include "EngineComponents.h"
#include "Time/Time.h"
#include "Renderer/ImGUIMain.h"
#include "Scripting/CSMonoCore.h"
#include "Components/TransformComponent.h"
#include "Components/SpriteComponent.h"
#include "Scripting/Objects/GameObjectInterface.h"
#include "Components/ComponentInterface.h"
#include "Asset/AssetManager.h"
#include "Scripting/Objects/RenderInterface.h"
#include "Components/ScriptComponent.h"
#include "Scripting/ScriptingManager.h"
#include "Input/Keyboard.h"
#include "Input/Input.h"
#include "Input/Mouse.h"
#include "Components/CameraComponent.h"
#include "Editor/EditorCore.h"
#include "SceneSystem/SceneLoader.h"
#include "Physics/PhysicsCore.h"
#include "Event/EventCore.h"
#include "Scripting/Objects/SceneInterface.h"
#include "Components/BoxColliderComponent.h"
#include "Components/DynamicBodyComponent.h"
#include "Components/StaticBodyComponent.h"
#include "Components/CircleColliderComponent.h"
#include "SceneSystem/GlobalScene.h"

RenderCore* render_core;
SceneManager* scene_manager;
CSMonoCore* mono_core;
SceneIndex main_scene;
AssetManager* asset_manager;
ScriptingManager* scripting_manager;
Keyboard* keyboard;
Mouse* mouse;
EditorCore* editor_core;
SceneLoader* scene_loader;
PhysicsCore* physics_core;
EventCore* event_core;
GlobalScene* global_scene;

void QREntryPoint::EntryPoint()
{
	event_core = new EventCore();

	asset_manager = new AssetManager();

	mono_core = new CSMonoCore();

	scripting_manager = new ScriptingManager();

	keyboard = new Keyboard();

	mouse = new Mouse();

	auto main_class_handle = mono_core->RegisterMonoClass("ScriptProject", "Main");

	render_core = new RenderCore(1920, 1080, L"2DRENDERER");

	scene_manager = new SceneManager();
	scene_loader = new SceneLoader();
	global_scene = new GlobalScene();

	main_scene = scene_manager->CreateScene();
	scene_manager->ChangeScene(main_scene);
	scene_manager->HandleDeferredScenes();

	auto main_method_handle = mono_core->RegisterMonoMethod(main_class_handle, "main");

	auto scene_manager_handle = mono_core->RegisterMonoClass("ScriptProject.Engine", "SceneManager");
	mono_core->HookAndRegisterMonoMethodType<SceneManager::GetActiveSceneIndex>(scene_manager_handle, "GetActiveSceneIndex", SceneManager::GetActiveSceneIndex);
	auto entity_manager_handle = mono_core->RegisterMonoClass("ScriptProject.Engine", "EntityManager");
	mono_core->HookAndRegisterMonoMethodType<EntityManager::CreateEntity>(entity_manager_handle, "CreateEntity", EntityManager::CreateEntity);

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

#ifdef _EDITOR
	editor_core = new EditorCore();
#else
	DrawScene::SetAddUserPrefab();
	//Temp
	EntityManager* global_entity_manager = scene_manager->GetEntityManager(global_scene->Get()->GetSceneIndex());
	auto m_editor_camera_ent = global_entity_manager->NewEntity();
	global_entity_manager->AddComponent<TransformComponent>(m_editor_camera_ent, Vector3(0.0f, 0.0f, 50.0f));
	global_entity_manager->AddComponent<CameraComponent>(m_editor_camera_ent);
#endif // _EDITOR

	physics_core = new PhysicsCore(true);

	mono_core->CallStaticMethod(main_method_handle);
}

Timer rendering_timer;
double average_rendering_frame_time = 0.0;
float average_frame_time = 0;
int frame_count = 0;
void QREntryPoint::RunTime()
{
	EntityManager* global_entity_manager = scene_manager->GetEntityManager(global_scene->Get()->GetSceneIndex());
	Entity player = global_entity_manager->NewEntity();
	global_entity_manager->AddComponent<TransformComponent>(player, Vector3(0.0f, 0.0f, 1.0f));
	global_entity_manager->AddComponent<SpriteComponent>(player).texture_handle = render_core->LoadTexture("../QRGameEngine/Textures/Temp.png");

	bool window_exist = true;
	while (window_exist)
	{
		Time::Start();

		Mouse::Get()->ResetMouseDeltaCoords();

		ImGUIMain::StartFrame();

		Scene* active_scene = scene_manager->GetScene(scene_manager->GetActiveSceneIndex());
		EntityManager* entman = active_scene->GetEntityManager();

		average_frame_time = average_frame_time * 0.9f + 0.1f * (float)Time::GetDeltaTime(Timer::TimeTypes::Milliseconds);
		float average_fps = 1000.0f / average_frame_time;

		ImGui::Begin("App Statistics");
		{
			ImGui::Text("Average Frame Time: %f ms", average_frame_time);
			ImGui::Text("Average Frame Per Second: %f", average_fps);
			ImGui::Text("Average Rendering Time: %f ms", average_rendering_frame_time);
			//ImGui::Text("Camera Position: x = %f, y = %f, z = %f", editor_camera_position.x, editor_camera_position.y, editor_camera_position.z);
		}
		ImGui::End();

		if (keyboard->GetKeyPressed(Keyboard::Key::I))
		{
			scene_manager->DestroyScene(scene_manager->GetActiveSceneIndex());
			SceneIndex scene = scene_manager->LoadScene("t1");
			scene_manager->ChangeScene(scene);
		}

#ifdef _EDITOR
		editor_core->Update();
#endif // _EDITOR

		/*
		* NOTE
		* If the user changes the scene to another then we should wait until the next frame to change and not change during!!!
		*/

		PhysicsCore::Get()->WaitForPhysics();
		PhysicsCore::Get()->HandleDeferredPhysicData();
		PhysicsCore::Get()->GetWorldPhysicObjectData(entman);
		PhysicsCore::Get()->GetWorldPhysicObjectData(global_entity_manager);

		//Update scripts
		ScriptingManager::Get()->UpdateScripts(entman);
		ScriptingManager::Get()->UpdateScripts(global_entity_manager);

		PhysicsCore::Get()->DrawColliders();

		PhysicsCore::Get()->SetWorldPhysicObjectData(entman);
		PhysicsCore::Get()->SetWorldPhysicObjectData(global_entity_manager);

		PhysicsCore::Get()->UpdatePhysics();

		keyboard->UpdateKeys();
		mouse->UpdateMouseButtons();

		rendering_timer.StartTimer();
		window_exist = render_core->UpdateRender(active_scene);
		if (!window_exist)
			break;
		average_rendering_frame_time = average_rendering_frame_time * 0.9 + 0.1 * rendering_timer.StopTimer() / (double)Timer::TimeTypes::Milliseconds;

		physics_core->RemoveDeferredPhysicObjects(entman);
		scripting_manager->RemoveDeferredScripts(entman);
		entman->DestroyDeferredEntities();
		physics_core->RemoveDeferredPhysicObjects(global_entity_manager);
		scripting_manager->RemoveDeferredScripts(global_entity_manager);
		global_entity_manager->DestroyDeferredEntities();

		scene_manager->HandleDeferredScenes();

		Time::Stop();
	}

	delete mono_core;
	delete render_core;
	delete scene_manager;
}
