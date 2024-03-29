#include "pch.h"
#include "QREntryPoint.h"
#include "Renderer/RenderCore.h"
#include "ECS/EntityManager.h"
#include "SceneSystem/SceneManager.h"
#include "EngineComponents.h"
#include "Time/Time.h"
#include "Renderer/ImGUIMain.h"
#include "Scripting/CSMonoCore.h"
#include "Asset/AssetManager.h"
#include "Scripting/ScriptingManager.h"
#include "Editor/EditorCore.h"
#include "SceneSystem/SceneLoader.h"
#include "Physics/PhysicsCore.h"
#include "Event/EventCore.h"
#include "SceneSystem/GlobalScene.h"
#include "SceneSystem/SceneHierarchy.h"
#include "Animation/AnimationManager.h"
#include "RegisterInterfaces.h"
#include "Input/Keyboard.h"
#include "Input/Input.h"
#include "Input/Mouse.h"
#include "Components/CameraComponent.h"
#include "PathFinding.h"
#include "Components/PathFindingWorldComponent.h"
#include "Scripting/Objects/GameObjectInterface.h"

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
SceneHierarchy* scene_hierarchy;
AnimationManager* animation_manager;
PathFinding* path_finding;

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
	scene_hierarchy = new SceneHierarchy();
	animation_manager = new AnimationManager();

	main_scene = scene_manager->CreateScene();
	scene_manager->ChangeScene(main_scene);
	scene_manager->HandleDeferredScenes();

	auto main_method_handle = mono_core->RegisterMonoMethod(main_class_handle, "main");

	auto scene_manager_handle = mono_core->RegisterMonoClass("ScriptProject.Engine", "SceneManager");
	mono_core->HookAndRegisterMonoMethodType<SceneManager::GetActiveSceneIndex>(scene_manager_handle, "GetActiveSceneIndex", SceneManager::GetActiveSceneIndex);
	auto entity_manager_handle = mono_core->RegisterMonoClass("ScriptProject.Engine", "EntityManager");
	mono_core->HookAndRegisterMonoMethodType<EntityManager::CreateEntity>(entity_manager_handle, "CreateEntity", EntityManager::CreateEntity);

	RegisterInterfaces::Register(mono_core);

#ifdef _EDITOR
	editor_core = new EditorCore();
#else
	DrawScene::SetAddUserPrefab();
	//Temp
	EntityManager* global_entity_manager = scene_manager->GetEntityManager(global_scene->Get()->GetSceneIndex());
	auto m_editor_camera_ent = global_entity_manager->NewEntity();
	global_entity_manager->AddComponent<TransformComponent>(m_editor_camera_ent, Vector3(0.0f, 0.0f, 20.0f));
	//global_entity_manager->AddComponent<CameraComponent>(m_editor_camera_ent);
#endif // _EDITOR

	physics_core = new PhysicsCore(true);

	mono_core->CallStaticMethod(main_method_handle);

	path_finding = new PathFinding();
}

Timer rendering_timer;
Timer scripting_timer;
Timer physic_timer;
double average_rendering_frame_time = 0.0;
double average_scripting_frame_time = 0.0;
double average_physic_frame_time = 0.0;
float average_frame_time = 0;
int frame_count = 0;

Entity path_finder = NULL_ENTITY;
bool change_scene = false;
void QREntryPoint::RunTime()
{
	EntityManager* global_entity_manager = scene_manager->GetEntityManager(global_scene->Get()->GetSceneIndex());

	bool window_exist = true;
	while (window_exist)
	{
		Time::Start();
		Timer time;

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
			ImGui::Text("Average Scripting Time: %f ms", average_scripting_frame_time);
			ImGui::Text("Average Physic Time: %f ms", average_physic_frame_time);
			//ImGui::Text("Camera Position: x = %f, y = %f, z = %f", editor_camera_position.x, editor_camera_position.y, editor_camera_position.z);
		}
		ImGui::End();

#ifndef _EDITOR
		//if (path_finder != NULL_ENTITY || change_scene)
		//{
		//	if (change_scene)
		//	{
		//		path_finding->ConstructPathFindingWorld(active_scene->GetSceneIndex());
		//		path_finder = entman->NewEntity();
		//		entman->AddComponent<TransformComponent>(path_finder).SetPositionZ(1);
		//		entman->AddComponent<SpriteComponent>(path_finder).texture_handle = RenderCore::Get()->LoadTexture("../QRGameEngine/Textures/Temp_2.png");
		//		change_scene = false;
		//	}
		//	Entity player = GameObjectInterface::TempFindGameObjectEntity("Player");
		//	std::vector<Entity> path = path_finding->PathFind(active_scene->GetSceneIndex(), path_finder, player);
		//	for (int i = 0; i < path.size(); ++i)
		//	{
		//		const TransformComponent& trans = entman->GetComponent<TransformComponent>(path[i]);
		//		if (i + 1 < path.size())
		//		{
		//			render_core->AddLine(Vector2(trans.GetPosition().x, trans.GetPosition().y));
		//			const TransformComponent& trans_next = entman->GetComponent<TransformComponent>(path[i+1]);
		//			render_core->AddLine(Vector2(trans_next.GetPosition().x, trans_next.GetPosition().y));
		//		}
		//	}
		//	TransformComponent& path_finder_trans = entman->GetComponent<TransformComponent>(path_finder);
		//	const Vector2 path_finder_pos = Vector2(path_finder_trans.GetPosition().x, path_finder_trans.GetPosition().y);
		//	Vector2 dir;
		//	if (path.size() > 2)
		//	{
		//		const TransformComponent& trans = entman->GetComponent<TransformComponent>(path[1]);
		//		dir = (Vector2(trans.GetPosition().x, trans.GetPosition().y) - path_finder_pos).Normalize();
		//	}
		//	else
		//	{
		//		const TransformComponent& trans = entman->GetComponent<TransformComponent>(player);
		//		dir = (Vector2(trans.GetPosition().x, trans.GetPosition().y) - path_finder_pos).Normalize();
		//	}
		//	Vector2 new_pos = path_finder_pos + Vector2(Time::GetDeltaTime(), Time::GetDeltaTime()) * dir;
		//	path_finder_trans.SetPosition(new_pos);
		//}
		if (keyboard->GetKeyPressed(Keyboard::Key::I))
		{
			scene_manager->DestroyScene(scene_manager->GetActiveSceneIndex());
			SceneIndex scene = scene_manager->LoadScene("port"); //"port_path_test_2"
			scene_manager->ChangeScene(scene);
			change_scene = false;
		}
#endif // EDITOR

#ifdef _EDITOR
		editor_core->Update();
#endif // _EDITOR

		/*
		* NOTE
		* If the user changes the scene to another then we should wait until the next frame to change and not change during!!!
		*/

		PhysicsCore::Get()->WaitForPhysics();
		PhysicsCore::Get()->HandleDeferredPhysicData();
		physic_timer.StartTimer();
		PhysicsCore::Get()->GetWorldPhysicObjectData(entman);
		PhysicsCore::Get()->GetWorldPhysicObjectData(global_entity_manager);
		PhysicsCore::Get()->HandleDeferredCollisionData();
		double physic_time_1 = physic_timer.StopTimer() / (double)Timer::TimeTypes::Milliseconds;
		//Update scripts
#ifndef _EDITOR
		scripting_timer.StartTimer();
		ScriptingManager::Get()->UpdateScripts(entman);
		ScriptingManager::Get()->UpdateScripts(global_entity_manager);
		average_scripting_frame_time = average_scripting_frame_time * 0.9 + 0.1 * scripting_timer.StopTimer() / (double)Timer::TimeTypes::Milliseconds;
#endif // EDITOR

		scene_hierarchy->UpdateEntityTransforms(active_scene->GetSceneIndex());

		physic_timer.StartTimer();
		PhysicsCore::Get()->DrawColliders();

		PhysicsCore::Get()->SetWorldPhysicObjectData(entman);
		PhysicsCore::Get()->SetWorldPhysicObjectData(global_entity_manager);

		PhysicsCore::Get()->UpdatePhysics();

		average_physic_frame_time = average_physic_frame_time * 0.9 + 0.1 * (physic_timer.StopTimer() / (double)Timer::TimeTypes::Milliseconds + physic_time_1);

		keyboard->UpdateKeys();
		mouse->UpdateMouseButtons();

		rendering_timer.StartTimer();
		window_exist = render_core->UpdateRender(active_scene);
		if (!window_exist)
			break;
		average_rendering_frame_time = average_rendering_frame_time * 0.9 + 0.1 * rendering_timer.StopTimer() / (double)Timer::TimeTypes::Milliseconds;

		physics_core->RemoveDeferredPhysicObjects(entman);
		scripting_manager->RemoveDeferredScripts(entman);
		scene_hierarchy->RemoveDeferredRelations(entman);
		entman->DestroyDeferredEntities();
		physics_core->RemoveDeferredPhysicObjects(global_entity_manager);
		scripting_manager->RemoveDeferredScripts(global_entity_manager);
		scene_hierarchy->RemoveDeferredRelations(global_entity_manager);
		global_entity_manager->DestroyDeferredEntities();

		scene_manager->HandleDeferredScenes();

		//double frame_time = 0.0f;
		//while (frame_time < 1000.0/60.0)
		//{
		//	frame_time = time.StopTimer() / (double)Timer::TimeTypes::Milliseconds;
		//}

		Time::Stop();
	}

	delete mono_core;
	delete render_core;
	delete scene_manager;
}
