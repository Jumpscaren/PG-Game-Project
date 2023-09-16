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

RenderCore* render_core;
SceneManager* scene_manager;
CSMonoCore* mono_core;
SceneIndex main_scene;
Entity render_ent;
AssetManager* asset_manager;
ScriptingManager* scripting_manager;
Keyboard* keyboard;
Mouse* mouse;
EditorCore* editor_core;
SceneLoader* scene_loader;
PhysicsCore* physics_core;

struct TempData
{
	TempData(int mm)
	{
		std::cout << "Hello\n";
		mine = mm * 2;
	}
	int mine;
};

struct TempComp
{
	int mine;
};

void PrintText()
{
	std::cout << "Text\n";
}

MonoMethodHandle return_int_method;
int CallReturnInt(float num1, double num2, std::string text, CSMonoObject me)
{
	//int int_num1 = *(int*)num1;
	//uint32_t 
	//float* numt = (float*)num1;
	//float g = (float)numt;
	//float float_num1 = (float)num1;

	//mono_core->MonoObjectToValue((std::string*)text);
	//mono_core->MonoObjectToValue((CSMonoObject**)&me);

	int return_int = 0;
	mono_core->CallMethod(return_int, return_int_method, me, 2.31f, 3.14, "Mannen!", me);
	return return_int;
}

int TESTING(int h)
{
	return h * 2;
}

void QREntryPoint::EntryPoint()
{
	asset_manager = new AssetManager();

	mono_core = new CSMonoCore();

	scripting_manager = new ScriptingManager();

	keyboard = new Keyboard();

	mouse = new Mouse();

	auto main_class_handle = mono_core->RegisterMonoClass("ScriptProject", "Main");

	auto print_method_handle = mono_core->HookAndRegisterMonoMethod(main_class_handle, "PrintText", &PrintText);
	mono_core->CallStaticMethod(print_method_handle);

	auto testing_func = mono_core->HookAndRegisterMonoMethodType<(void*)TESTING>(main_class_handle, "Testing", &TESTING);
	mono_core->CallStaticMethod(testing_func, 4);
	/*mono_core->CallMethod(call_return_int_handle, nullptr, 4);*/

	auto call_return_int_handle = mono_core->HookAndRegisterMonoMethodType<CallReturnInt>(main_class_handle, "CallReturnInt", &CallReturnInt);

	auto testfunc_method_handle = mono_core->RegisterMonoMethod(main_class_handle, "TestFunc");

	CSMonoObject object(mono_core, main_class_handle);

	auto print_args_method = mono_core->RegisterMonoMethod(main_class_handle, "PrintArgs");
	mono_core->CallMethod(print_args_method, object, 10, false, "Text");

	return_int_method = mono_core->RegisterMonoMethod(main_class_handle, "ReturnInt");
	int return_int;
	mono_core->CallMethod(return_int, return_int_method, object, 2.31f, 3.14, "Mannen!", object);

	mono_core->CallMethod(testfunc_method_handle, object);
	object.CallMethod(testfunc_method_handle);
	mono_core->CallStaticMethod(testfunc_method_handle);

	CSMonoObject this_object;
	mono_core->GetValue(this_object, object, "i");
	mono_core->SetValue(this_object, object, "i");
	mono_core->GetValue(return_int, object, "num");
	mono_core->SetValue(return_int, object, "num");
	auto mono_i_handle = mono_core->RegisterField(main_class_handle, "i");
	auto mono_num_handle = mono_core->RegisterField(main_class_handle, "num");
	mono_core->GetValue(this_object, object, mono_i_handle);
	mono_core->SetValue(this_object, object, mono_i_handle);
	mono_core->GetValue(return_int, object, mono_num_handle);
	mono_core->SetValue(return_int, object, mono_num_handle);

	auto test_script_class = mono_core->RegisterMonoClass("ScriptProject", "TestScript");
	auto start_method = mono_core->RegisterMonoMethod(test_script_class, "Start");
	auto update_method = mono_core->TryRegisterMonoMethod(test_script_class, "Update");

	//assert(mono_core->CheckIfMonoMethodExists(update_method));

	EntityManager ent(100);
	Entity e = ent.NewEntity();
	TempData& data = ent.AddComponent<TempData>(e, 2);
	++data.mine;

	bool check = ent.HasComponent<TempData>(e);
	check = ent.HasComponent<TempComp>(e);

	data = ent.GetComponent<TempData>(e);
	//Crashes as intended
	//ent.GetComponent<TempComp>(e);

	ent.System<TempData>([&](TempData& mdata)
		{
			std::cout << mdata.mine << "\n";
		});

	ent.System<TempData>([&](Entity e, TempData& mdata)
		{
			std::cout << e << " GG " << mdata.mine << "\n";
		});

	ent.AddComponent<TempComp>(e);
	ent.RemoveComponent<TempComp>(e);
	ent.AddComponent<TempComp>(e);

	ent.RemoveEntity(e);

	std::cout << "h\n";

	render_core = new RenderCore(1920, 1080, L"2DRENDERER");

	scene_manager = new SceneManager();
	scene_loader = new SceneLoader();

	main_scene = scene_manager->CreateScene();
	scene_manager->SetSceneAsActiveScene(main_scene);

	TextureHandle texture = render_core->LoadTexture("../QRGameEngine/Textures/Temp.png");

	EntityManager* em = scene_manager->GetScene(main_scene)->GetEntityManager();
	render_ent = em->NewEntity();
	em->AddComponent<TransformComponent>(render_ent, Vector3(0.0f, 0.0f, 1.0f), Vector3(0.0f, 0.0f, DirectX::XM_PIDIV4 / 2.0f), Vector3(0.2f, 0.2f, 0.2f));
	em->AddComponent<SpriteComponent>(render_ent).texture_handle = texture;

	render_ent = em->NewEntity();
	em->AddComponent<TransformComponent>(render_ent, Vector3(0.2f, 0.0f, 1.0f), Vector3(0.0f, 0.0f, DirectX::XM_PIDIV4 / 1.0f), Vector3(0.2f, 0.2f, 0.2f));
	em->AddComponent<SpriteComponent>(render_ent).texture_handle = texture;

	render_ent = em->NewEntity();
	em->AddComponent<TransformComponent>(render_ent, Vector3(0.4f, 0.0f, 1.0f), Vector3(0.0f, 0.0f, DirectX::XM_PIDIV4 / 1.5f), Vector3(0.2f, 0.2f, 0.2f));
	em->AddComponent<SpriteComponent>(render_ent).texture_handle = texture;

	render_ent = em->NewEntity();
	em->AddComponent<TransformComponent>(render_ent, Vector3(0.6f, 0.0f, 1.0f), Vector3(0.0f, 0.0f, DirectX::XM_PIDIV4 / 0.5f), Vector3(0.2f, 0.2f, 0.2f));
	em->AddComponent<SpriteComponent>(render_ent).texture_handle = texture;

	auto main_method_handle = mono_core->RegisterMonoMethod(main_class_handle, "main");

	auto scene_manager_handle = mono_core->RegisterMonoClass("ScriptProject.Engine", "SceneManager");
	mono_core->HookAndRegisterMonoMethodType<SceneManager::GetActiveSceneIndex>(scene_manager_handle, "GetActiveSceneIndex", SceneManager::GetActiveSceneIndex);
	auto entity_manager_handle = mono_core->RegisterMonoClass("ScriptProject.Engine", "EntityManager");
	mono_core->HookAndRegisterMonoMethodType<EntityManager::CreateEntity>(entity_manager_handle, "CreateEntity", EntityManager::CreateEntity);

	TransformComponentInterface::RegisterInterface(mono_core);
	SpriteComponentInterface::RegisterInterface(mono_core);
	GameObjectInterface::RegisterInterface(mono_core);
	ComponentInterface::RegisterInterface(mono_core);
	RenderInterface::RegisterInterface(mono_core);
	ScriptComponentInterface::RegisterInterface(mono_core);
	InputInterface::RegisterInterface(mono_core);
	CameraComponentInterface::RegisterInterface(mono_core);

#ifdef _EDITOR
	editor_core = new EditorCore();
#endif // _EDITOR

	mono_core->CallStaticMethod(main_method_handle);

	physics_core = new PhysicsCore();

	//Testcase 1
	PhysicsCore::Get()->AddCirclePhysicObject(em, render_ent, PhysicsCore::PhysicObjectBodyType::DynamicBody, 0.5f);
	PhysicsCore::Get()->AddBoxCollider(em, render_ent, Vector2(0.5f, 0.5f));
	PhysicsCore::Get()->RemoveBoxCollider(em, render_ent);
	PhysicsCore::Get()->RemoveCircleCollider(em, render_ent);
	PhysicsCore::Get()->RemovePhysicObject(em, render_ent);

	//Testcase 2
	PhysicsCore::Get()->AddBoxPhysicObject(em, render_ent, PhysicsCore::PhysicObjectBodyType::DynamicBody, Vector2(0.5f, 0.5f));
	PhysicsCore::Get()->AddCircleCollider(em, render_ent, 0.5f);
	PhysicsCore::Get()->RemoveCircleCollider(em, render_ent);
	PhysicsCore::Get()->RemoveBoxCollider(em, render_ent);
	PhysicsCore::Get()->RemovePhysicObject(em, render_ent);

	//Testcase 3
	PhysicsCore::Get()->AddPhysicObject(em, render_ent, PhysicsCore::PhysicObjectBodyType::DynamicBody);
	PhysicsCore::Get()->AddBoxCollider(em, render_ent, Vector2(0.5f, 0.5f));
	PhysicsCore::Get()->AddCircleCollider(em, render_ent, 0.5f);
	PhysicsCore::Get()->RemoveCircleCollider(em, render_ent);
	PhysicsCore::Get()->RemoveBoxCollider(em, render_ent);
	PhysicsCore::Get()->RemovePhysicObject(em, render_ent);

	//Testcase 4
	PhysicsCore::Get()->AddPhysicObject(em, render_ent, PhysicsCore::PhysicObjectBodyType::DynamicBody);
	PhysicsCore::Get()->AddCircleCollider(em, render_ent, 0.5f);
	PhysicsCore::Get()->RemoveCircleCollider(em, render_ent);
	PhysicsCore::Get()->AddBoxCollider(em, render_ent, Vector2(0.5f, 0.5f));
	PhysicsCore::Get()->RemoveBoxCollider(em, render_ent);
	PhysicsCore::Get()->RemovePhysicObject(em, render_ent);

	//Testcase 5
	PhysicsCore::Get()->AddPhysicObject(em, render_ent, PhysicsCore::PhysicObjectBodyType::DynamicBody);
	PhysicsCore::Get()->AddBoxCollider(em, render_ent, Vector2(0.5f, 0.5f));
	PhysicsCore::Get()->AddCircleCollider(em, render_ent, 0.5f);
	PhysicsCore::Get()->RemovePhysicObject(em, render_ent);

	PhysicsCore::Get()->AddBoxPhysicObject(em, render_ent, PhysicsCore::PhysicObjectBodyType::DynamicBody, Vector2(0.5f, 0.5f));

	auto ground_ent = em->NewEntity();
	em->AddComponent<TransformComponent>(ground_ent, Vector3(0.0f, -5.0f, 0.0f));
	PhysicsCore::Get()->AddBoxPhysicObject(em, ground_ent, PhysicsCore::PhysicObjectBodyType::StaticBody, Vector2(20.0f, 1.0f));

	auto trigger_ent = em->NewEntity();
	em->AddComponent<TransformComponent>(trigger_ent, Vector3(0.0f, 5.0f, 0.0f));
	PhysicsCore::Get()->AddBoxPhysicObject(em, trigger_ent, PhysicsCore::PhysicObjectBodyType::DynamicBody, Vector2(0.5f, 0.5f));

	auto circle_ent = em->NewEntity();
	em->AddComponent<TransformComponent>(circle_ent, Vector3(0.0f, 3.0f, 0.0f));
	PhysicsCore::Get()->AddCirclePhysicObject(em, circle_ent, PhysicsCore::PhysicObjectBodyType::DynamicBody, 1.0f);

	PhysicsCore::Get()->testg();
}

float average_frame_time = 0;
void QREntryPoint::RunTime()
{
	bool window_exist = true;
	while (window_exist)
	{
		Time::Start();

		Mouse::Get()->ResetMouseDeltaCoords();

		ImGUIMain::StartFrame();

		Scene* active_scene = scene_manager->GetScene(main_scene);
		EntityManager* entman = active_scene->GetEntityManager();

		average_frame_time = average_frame_time * 0.9f + 0.1f * (float)Time::GetDeltaTime(Timer::TimeTypes::Milliseconds);
		float average_fps = 1000.0f / average_frame_time;

		ImGui::Begin("App Statistics");
		{
			ImGui::Text("Average Frame Time: %f ms", average_frame_time);
			ImGui::Text("Average Frame Per Second: %f", average_fps);
			//ImGui::Text("Camera Position: x = %f, y = %f, z = %f", editor_camera_position.x, editor_camera_position.y, editor_camera_position.z);
		}
		ImGui::End();

		TransformComponent& render_ent_trans = entman->GetComponent<TransformComponent>(render_ent);
		
		Vector3 pos = render_ent_trans.GetPosition();
		pos.x += (float)Time::GetDeltaTime();
		render_ent_trans.SetPosition(pos);

		Vector3 rot = render_ent_trans.GetRotationEuler();

#ifdef _EDITOR
		editor_core->Update();
#endif // _EDITOR

		/*
		* NOTE
		* If the user changes the scene to another then we should wait until the next frame to change and not change during!!!
		*/

		//Update scripts
		ScriptingManager::Get()->UpdateScripts(entman);

		PhysicsCore::Get()->SetWorldPhysicObjectData(entman);
		PhysicsCore::Get()->Update();
		PhysicsCore::Get()->GetWorldPhysicObjectData(entman);

		keyboard->UpdateKeys();
		mouse->UpdateMouseButtons();

		window_exist = render_core->UpdateRender(scene_manager->GetScene(main_scene));
		if (!window_exist)
			break;

		physics_core->RemoveDeferredPhysicObjects(entman);
		scripting_manager->RemoveDeferredScripts(entman);
		entman->DestroyDeferredEntities();

		Time::Stop();
	}

	delete mono_core;
	delete render_core;
	delete scene_manager;
}
