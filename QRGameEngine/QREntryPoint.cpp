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
#include "PathFinding/PathFinding.h"
#include "Components/PathFindingWorldComponent.h"
#include "Scripting/Objects/GameObjectInterface.h"
#include "Scripting/Objects/TimeInterface.h"
#include "ECS/ComponentMap.h"

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

float f = 0.0;
int i = 0;
void cpptest()
{
	f += ++i;
}
void cpptestint(const int i)
{
	f += i;
}
void cpptestmany(const CSMonoObject& object)
{
	
}

struct Data {
	int num;
	float other;
	double big;
	char tecken;
};

int square(const Data& num, const Data& num2, const Data& num3) {
	return num.num * num2.num * num3.num;
}

template <typename ...Args>
int squarenum(Args&&... args)
{
	int num = square(std::forward<Args>(args)...);
	int num2 = square(std::forward<Args>(args)...);
	//int num2 = square(args...);

	return num + num2;
}

void QREntryPoint::EntryPoint()
{
	Data data{};
	data.num = 1;
	squarenum(data, Data{ .num = 2 }, Data{ .num = 3 });

	ComponentMap::AddAllComponents();

	event_core = new EventCore();

	asset_manager = new AssetManager();

	mono_core = new CSMonoCore();

	scripting_manager = new ScriptingManager();

	keyboard = new Keyboard();

	mouse = new Mouse();

	scene_hierarchy = new SceneHierarchy();
	scene_manager = new SceneManager();
	scene_loader = new SceneLoader();
	global_scene = new GlobalScene();

	//render_core = new RenderCore(1920, 1080, L"2DRENDERER");
	render_core = new RenderCore(1280, 720, L"2DRENDERER");
	//render_core = new RenderCore(480, 360, L"2DRENDERER");

	animation_manager = new AnimationManager();

	path_finding = new PathFinding();

	main_scene = scene_manager->CreateScene();
	scene_manager->ForceChangeScene(main_scene);
	scene_manager->HandleDeferredScenes();

	auto main_class_handle = mono_core->RegisterMonoClass("ScriptProject", "Main");
	auto main_method_handle = mono_core->RegisterMonoMethod(main_class_handle, "main");

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

	auto test1 = mono_core->RegisterMonoMethod(main_class_handle, "test1");
	auto test2 = mono_core->RegisterMonoMethod(main_class_handle, "test2");
	auto test3 = mono_core->RegisterMonoMethod(main_class_handle, "test3");
	auto test4 = mono_core->RegisterMonoMethod(main_class_handle, "test4");
	mono_core->HookAndRegisterMonoMethodType<cpptest>(main_class_handle, "cpptest", cpptest);
	mono_core->HookAndRegisterMonoMethodType<cpptestint>(main_class_handle, "cpptestint", cpptestint);
	mono_core->HookAndRegisterMonoMethodType<cpptestmany>(main_class_handle, "cpptestmany", cpptestmany);

	//Timer time;
	//mono_core->CallStaticMethod(test1);
	//auto testTime1 = time.StopTimer();

	//time.StartTimer();
	//mono_core->CallStaticMethod(test2);
	//auto testTime2 = time.StopTimer();

	//time.StartTimer();
	//mono_core->CallStaticMethod(test3);
	//auto testTime3 = time.StopTimer();

	//mono_core->CallStaticMethod(test4);
	//time.StartTimer();
	//mono_core->CallStaticMethod(test4);
	//auto testTime4 = time.StopTimer();

	//std::cout << "Time 1: " << testTime1 / (double)Timer::TimeTypes::Milliseconds << "\nTime 2: " << testTime2 / (double)Timer::TimeTypes::Milliseconds << "\nTime 3: " 
	//	<< testTime3 / (double)Timer::TimeTypes::Milliseconds << "\nTime 4: " << testTime4 / (double)Timer::TimeTypes::Milliseconds << "\n";
}

Timer rendering_timer;
Timer scripting_timer;
Timer physic_timer;
Timer physic_deferred_collision_timer;
Timer deferred_timer;
double average_rendering_frame_time = 0.0;
double average_scripting_frame_time = 0.0;
double average_physic_frame_time = 0.0;
double average_physics_deferred_collision_time = 0.0;
double average_deferred_frame_time = 0.0;
float average_frame_time = 0;
int frame_count = 0;

Entity path_finder = NULL_ENTITY;
bool change_scene = false;
void QREntryPoint::RunTime()
{
	EntityManager* global_entity_manager = scene_manager->GetEntityManager(global_scene->Get()->GetSceneIndex());

	const auto orc_enemy_class_handle = mono_core->RegisterMonoClass("ScriptProject.Scripts", "OrcEnemy");
	const auto orc_enemy_get_count_method_handle = mono_core->RegisterMonoMethod(orc_enemy_class_handle, "GetCount");

	//auto trace = std::stacktrace::current();
	//for (const auto& entry : trace) {
	//	std::cout << "Description: " << entry.description() << std::endl;
	//	std::cout << "file: " << entry.source_file() << std::endl;
	//	std::cout << "line: " << entry.source_line() << std::endl;
	//	std::cout << "------------------------------------" << std::endl;
	//}

	auto* eventcore = EventCore::Get();

	bool window_exist = true;
	while (window_exist)
	{
		Time::Start();
		Timer time;
		TimeInterface::SetDeltaTime(mono_core);
		TimeInterface::SetElapsedTime(mono_core);

		Mouse::Get()->ResetMouseDeltaCoords();

		ImGUIMain::StartFrame();

		Scene* active_scene = scene_manager->GetScene(scene_manager->GetActiveSceneIndex());
		EntityManager* entman = active_scene->GetEntityManager();

		average_frame_time = average_frame_time * 0.9f + 0.1f * (float)Time::GetDeltaTime(Timer::TimeTypes::Milliseconds);
		float average_fps = 1000.0f / average_frame_time;

		int orc_count = 0;
		mono_core->CallStaticMethod(orc_count, orc_enemy_get_count_method_handle);
		ImGui::Begin("App Statistics");
		{
			ImGui::Text("Average Frame Time: %f ms", average_frame_time);
			ImGui::Text("Average Frame Per Second: %f", average_fps);
			ImGui::Text("Average Rendering Time: %f ms", average_rendering_frame_time);
			ImGui::Text("Average Scripting Time: %f ms", average_scripting_frame_time);
			ImGui::Text("Average Physic Time: %f ms", average_physic_frame_time);
			ImGui::Text("Average Physics Deferred Time Time: %f ms", average_physics_deferred_collision_time);
			ImGui::Text("Average Deferred Time: %f ms", average_deferred_frame_time);
			ImGui::Text("Window Width: %f, Window Height: %f", render_core->GetWindow()->GetWindowWidth(), render_core->GetWindow()->GetWindowHeight());
			ImGui::Text("Orcs Count: %i", orc_count);
			//ImGui::Text("Camera Position: x = %f, y = %f, z = %f", editor_camera_position.x, editor_camera_position.y, editor_camera_position.z);
		}
		ImGui::End();

#ifndef _EDITOR
		if (keyboard->GetKeyPressed(Keyboard::Key::I))
		{
			//scene_manager->DestroyScene(scene_manager->GetActiveSceneIndex());
			//SceneIndex scene = scene_manager->LoadScene("port"); //"port_path_test_2"
			//SceneIndex scene = scene_manager->LoadScene("test_area_1");
			std::cout << "Change Scene" << std::endl;
			if (scene_manager->GetScene(scene_manager->GetActiveSceneIndex())->GetSceneName() == "temp")
			{
				//SceneIndex scene = scene_manager->LoadScene("empty_with_camerad", true);
				SceneIndex scene = scene_manager->LoadScene("empty_with_camerad", true);
				scene_manager->ChangeScene(scene);
				change_scene = false;
			}
			else
			{
				SceneIndex scene = scene_manager->LoadScene("temp", true);
				scene_manager->ChangeScene(scene);
				change_scene = false;
			}
		}
#endif // EDITOR

#ifdef _EDITOR
		editor_core->Update();
#endif // _EDITOR

		/*
		* NOTE
		* If the user changes the scene to another then we should wait until the next frame to change and not change during!!!
		*/

		/*SceneLoader::Get()->HandleSceneLoadingPreUser();*/

		SceneLoader::Get()->HandleSceneLoadingPreUser();

		asset_manager->HandleCompletedJobs();

		physic_timer.StartTimer();
		PhysicsCore::Get()->WaitForPhysics();
		PhysicsCore::Get()->HandleDeferredPhysicData();
		PhysicsCore::Get()->GetWorldPhysicObjectData(entman);
		PhysicsCore::Get()->GetWorldPhysicObjectData(global_entity_manager);
		physic_deferred_collision_timer.StartTimer();
		PhysicsCore::Get()->HandleDeferredCollisionData();
		average_physics_deferred_collision_time = average_physics_deferred_collision_time * 0.9 + 0.1 * physic_deferred_collision_timer.StopTimer() / (double)Timer::TimeTypes::Milliseconds;
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

		PhysicsCore::Get()->SetWorldPhysicObjectData(entman);
		PhysicsCore::Get()->SetWorldPhysicObjectData(global_entity_manager);

		PhysicsCore::Get()->DrawColliders(entman);
		PhysicsCore::Get()->DrawColliders(global_entity_manager);

		PhysicsCore::Get()->UpdatePhysics();

		average_physic_frame_time = average_physic_frame_time * 0.9 + 0.1 * (physic_timer.StopTimer() / (double)Timer::TimeTypes::Milliseconds + physic_time_1);

		keyboard->UpdateKeys();
		mouse->UpdateMouseButtons();

#ifndef _EDITOR
		animation_manager->UpdateAnimations(entman);
		animation_manager->UpdateAnimations(global_entity_manager);
#endif // EDITOR

		rendering_timer.StartTimer();
		window_exist = render_core->UpdateRender(active_scene);
		if (!window_exist)
		{
			break;
		}
		average_rendering_frame_time = average_rendering_frame_time * 0.9 + 0.1 * rendering_timer.StopTimer() / (double)Timer::TimeTypes::Milliseconds;

		deferred_timer.StartTimer();
		//Entities can have been created after calling for destruction of a scene
		scene_manager->RemoveEntitiesFromDeferredDestroyedScenes();

		physics_core->RemoveDeferredPhysicObjects(entman);
		scripting_manager->RemoveDeferredScripts(entman);
		scene_hierarchy->RemoveDeferredRelations(entman);
		path_finding->HandleDeferredRemovedNodes(entman);
		GameObjectInterface::HandleDeferredEntities(entman);
		entman->DestroyDeferredEntities();
		physics_core->RemoveDeferredPhysicObjects(global_entity_manager);
		scripting_manager->RemoveDeferredScripts(global_entity_manager);
		scene_hierarchy->RemoveDeferredRelations(global_entity_manager);
		path_finding->HandleDeferredRemovedNodes(global_entity_manager);
		GameObjectInterface::HandleDeferredEntities(global_entity_manager);
		global_entity_manager->DestroyDeferredEntities();

		scene_manager->HandleDeferredScenes();

		//for (const auto g : CSMonoObject::test)
		//{
		//	std::cout << "Not removed: " << CSMonoObject::GetName(g) << "\n";
		//}
		//std::cout << "Size = " << CSMonoObject::test.size() << "\n";

		SceneLoader::Get()->HandleSceneLoadingPostUser();

		//mono_core->ForceGarbageCollection();

		average_deferred_frame_time = average_deferred_frame_time * 0.9 + 0.1 * deferred_timer.StopTimer() / (double)Timer::TimeTypes::Milliseconds;

		//double frame_time = 0.0f;
		//while (frame_time < 16.67)//1000.0 / 100.0)
		//{
		//	frame_time = time.StopTimer() / (double)Timer::TimeTypes::Milliseconds;
		//}

		Time::Stop();
	}
}

void QREntryPoint::Cleanup()
{
	delete scene_loader;
	delete render_core;
	delete scene_manager;
	delete mono_core;
	delete asset_manager;
	delete scripting_manager;
	delete keyboard;
	delete mouse;
	delete editor_core;
	delete physics_core;
	delete event_core;
	delete global_scene;
	delete scene_hierarchy;
	delete animation_manager;
	delete path_finding;
}
