#include "pch.h"
#include "QREntryPoint.h"
#include "Renderer/RenderCore.h"
#include "ECS/EntityManager.h"
#include "SceneSystem/SceneManager.h"
#include "EngineComponents.h"
#include "Time/Time.h"
#include "Renderer/ImGUIMain.h"
#include "Scripting/CSMonoCore.h"

RenderCore* render_core;
SceneManager* scene_manager;
CSMonoCore* mono_core;
SceneIndex main_scene;
Entity render_ent;

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
int CallReturnInt(float num1, double num2, CSMonoString* text, CSMonoObject* me)
{
	//int int_num1 = *(int*)num1;
	//uint32_t 
	//float* numt = (float*)num1;
	//float g = (float)numt;
	//float float_num1 = (float)num1;

	//mono_core->MonoObjectToValue((std::string*)text);
	//mono_core->MonoObjectToValue((CSMonoObject**)&me);

	int return_int = 0;
	std::string string_text = *text;
	//return_int = 
	/*mono_core->CallMethod(return_int, return_int_method, &me, 2.31f, 3.14, "PenisMannen!", &me);*/
	return return_int;
}

int TESTING(int h)
{
	return h * 2;
}

void QREntryPoint::EntryPoint()
{
	mono_core = new CSMonoCore();

	auto main_class_handle = mono_core->RegisterMonoClass("ScriptProject", "Main");
	auto main_method_handle = mono_core->RegisterMonoMethod(main_class_handle, "main");
	mono_core->CallMethod(main_method_handle);

	auto print_method_handle = mono_core->HookAndRegisterMonoMethod(main_class_handle, "PrintText", &PrintText);
	mono_core->CallMethod(print_method_handle);

	auto testing_func = mono_core->HookAndRegisterMonoMethodType<(void*)TESTING>(main_class_handle, "Testing", &TESTING);
	mono_core->CallMethod(testing_func, nullptr, 4);
	/*mono_core->CallMethod(call_return_int_handle, nullptr, 4);*/

	auto call_return_int_handle = mono_core->HookAndRegisterMonoMethodType<(void*)CallReturnInt>(main_class_handle, "CallReturnInt", &CallReturnInt);

	auto testfunc_method_handle = mono_core->RegisterMonoMethod(main_class_handle, "TestFunc");

	CSMonoObject object(mono_core, main_class_handle);
	mono_core->CallMethod(testfunc_method_handle, &object);
	object.CallMethod(testfunc_method_handle);
	mono_core->CallMethod(testfunc_method_handle);

	auto print_args_method = mono_core->RegisterMonoMethod(main_class_handle, "PrintArgs");
	mono_core->CallMethod(print_args_method, &object, 10, false, "Text");

	return_int_method = mono_core->RegisterMonoMethod(main_class_handle, "ReturnInt");
	int return_int;
	mono_core->CallMethod(return_int, return_int_method, &object, 2.31f, 3.14, "PenisMannen!", &object);

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
	main_scene = scene_manager->CreateScene();
	scene_manager->SetSceneAsActiveScene(main_scene);

	TextureHandle texture = render_core->CreateTexture("../QRGameEngine/Textures/Temp.png");

	EntityManager* em = scene_manager->GetScene(main_scene)->GetEntityManager();
	render_ent = em->NewEntity();
	em->AddComponent<TransformComponent>(render_ent, Vector3(), Vector3(0.0f, 0.0f, DirectX::XM_PIDIV4 / 2.0f), Vector3(0.2f, 0.2f, 0.2f));
	em->AddComponent<SpriteComponent>(render_ent).texture_handle = texture;

	render_ent = em->NewEntity();
	em->AddComponent<TransformComponent>(render_ent, Vector3(0.2f), Vector3(0.0f, 0.0f, DirectX::XM_PIDIV4 / 1.0f), Vector3(0.2f, 0.2f, 0.2f));
	em->AddComponent<SpriteComponent>(render_ent).texture_handle = texture;

	render_ent = em->NewEntity();
	em->AddComponent<TransformComponent>(render_ent, Vector3(0.4f), Vector3(0.0f, 0.0f, DirectX::XM_PIDIV4 / 1.5f), Vector3(0.2f, 0.2f, 0.2f));
	em->AddComponent<SpriteComponent>(render_ent).texture_handle = texture;

	render_ent = em->NewEntity();
	em->AddComponent<TransformComponent>(render_ent, Vector3(0.6f), Vector3(0.0f, 0.0f, DirectX::XM_PIDIV4 / 0.5f), Vector3(0.2f, 0.2f, 0.2f));
	em->AddComponent<SpriteComponent>(render_ent).texture_handle = texture;
}

float average_frame_time = 0;
void QREntryPoint::RunTime()
{
	bool window_exist = true;
	while (window_exist)
	{
		Time::Start();

		ImGUIMain::StartFrame();

		average_frame_time = average_frame_time * 0.9f + 0.1f * (float)Time::GetDeltaTime(Timer::TimeTypes::Milliseconds);

		ImGui::Begin("App Statistics");
		{
			ImGui::Text("Average Frame Time: %f ms", average_frame_time);
		}
		ImGui::End();

		Scene* active_scene = scene_manager->GetScene(main_scene);
		EntityManager* entman = active_scene->GetEntityManager();

		TransformComponent& render_ent_trans = entman->GetComponent<TransformComponent>(render_ent);
		
		Vector3 pos = render_ent_trans.GetPosition();
		pos.x += (float)Time::GetDeltaTime();
		render_ent_trans.SetPosition(pos);


		window_exist = render_core->UpdateRender(scene_manager->GetScene(main_scene));
		if (!window_exist)
			break;

		Time::Stop();
	}

	delete mono_core;
	delete render_core;
	delete scene_manager;
}
