#include "pch.h"
#include "PolygonColliderComponent.h"
#include "Scripting/CSMonoCore.h"
#include "SceneSystem/SceneLoader.h"
#include "DynamicBodyComponent.h"
#include "StaticBodyComponent.h"
#include "PureStaticBodyComponent.h"
#include "Physics/PhysicsCore.h"
#include "SceneSystem/SceneManager.h"
#include "Scripting/Objects/GameObjectInterface.h"
#include "IO/JsonObject.h"
#include "KinematicBodyComponent.h"

DeferedMethodIndex PolygonColliderComponentInterface::s_add_physic_object_index;
DeferedMethodIndex PolygonColliderComponentInterface::s_add_polygon_collider_index;
DeferedMethodIndex PolygonColliderComponentInterface::s_remove_polygon_collider_index;

void PolygonColliderComponentInterface::RegisterInterface(CSMonoCore* mono_core, const DeferedMethodIndex add_physic_object_index, const DeferedMethodIndex add_polygon_collider_index, const DeferedMethodIndex remove_polygon_collider_index)
{
	auto polygon_collider_class = mono_core->RegisterMonoClass("ScriptProject.Engine", "PolygonCollider");

	mono_core->HookAndRegisterMonoMethodType<PolygonColliderComponentInterface::InitComponent>(polygon_collider_class, "InitComponent", PolygonColliderComponentInterface::InitComponent);
	mono_core->HookAndRegisterMonoMethodType<PolygonColliderComponentInterface::HasComponent>(polygon_collider_class, "HasComponent", PolygonColliderComponentInterface::HasComponent);
	mono_core->HookAndRegisterMonoMethodType<PolygonColliderComponentInterface::RemoveComponent>(polygon_collider_class, "RemoveComponent", PolygonColliderComponentInterface::RemoveComponent);

	mono_core->HookAndRegisterMonoMethodType<PolygonColliderComponentInterface::SetTrigger>(polygon_collider_class, "SetTrigger", PolygonColliderComponentInterface::SetTrigger);

	SceneLoader::Get()->OverrideSaveComponentMethod<PolygonColliderComponent>(SaveScriptComponent, LoadScriptComponent);

	s_add_physic_object_index = add_physic_object_index;
	s_add_polygon_collider_index = add_polygon_collider_index;
	s_remove_polygon_collider_index = remove_polygon_collider_index;
}

void PolygonColliderComponentInterface::InitComponent(const CSMonoObject& object, SceneIndex scene_index, Entity entity)
{
	EntityManager* entity_manager = SceneManager::GetSceneManager()->GetEntityManager(scene_index);
	SceneLoaderDeferCalls* defer_method_calls = SceneLoader::Get()->GetDeferedCalls();

	//So that we do not need add staticbody when adding a circlecollider if we do not use a dynamic body
	if (!entity_manager->HasComponent<DynamicBodyComponent>(entity) && !entity_manager->HasComponent<StaticBodyComponent>(entity) && !entity_manager->HasComponent<PureStaticBodyComponent>(entity) && !entity_manager->HasComponent<KinematicBodyComponent>(entity))
	{
		if (!defer_method_calls->TryCallDirectly(scene_index, s_add_physic_object_index, scene_index, entity, PhysicsCore::StaticBody))
		{
			SceneManager::GetSceneManager()->GetEntityManager(scene_index)->AddComponent<StaticBodyComponent>(entity);
		}
	}

	if (!defer_method_calls->TryCallDirectly(scene_index, s_add_polygon_collider_index, scene_index, entity, std::vector<Vector2>{ Vector2(0.5f, 0.5f), Vector2(-0.5f, -0.5f), Vector2(1.0f, 0.0f) }, true, true, false, ColliderFilter{}))
	{
		SceneManager::GetSceneManager()->GetEntityManager(scene_index)->AddComponent<PolygonColliderComponent>(entity).debug_draw = true;
	}
}

bool PolygonColliderComponentInterface::HasComponent(const CSMonoObject& object, SceneIndex scene_index, Entity entity)
{
	return SceneManager::GetSceneManager()->GetEntityManager(scene_index)->HasComponent<PolygonColliderComponent>(entity);
}

void PolygonColliderComponentInterface::RemoveComponent(const CSMonoObject& object, SceneIndex scene_index, Entity entity)
{
	SceneLoaderDeferCalls* defer_method_calls = SceneLoader::Get()->GetDeferedCalls();

	defer_method_calls->TryCallDirectly(scene_index, s_remove_polygon_collider_index, scene_index, entity);
}

bool IsInsideTriangle(const Vector3& a, const Vector3& b, const Vector3& c, const Vector3& q)
{
	const auto v0 = c - a;
	const auto v1 = b - a;
	const auto v2 = q - a;

	const auto dot00 = Vector3::Dot(v0, v0);
	const auto dot01 = Vector3::Dot(v0, v1);
	const auto dot02 = Vector3::Dot(v0, v2);
	const auto dot11 = Vector3::Dot(v1, v1);
	const auto dot12 = Vector3::Dot(v1, v2);

	const auto denom = dot00 * dot11 - dot01 * dot01;

	const auto zero = 1e-15;
	if (std::abs(denom) < zero)
	{
		return false;
	}

	const auto invDenom = 1.0 / denom;

	const auto ux = (dot11 * dot02 - dot01 * dot12) * invDenom;
	const auto vx = (dot00 * dot12 - dot01 * dot02) * invDenom;

	return (ux >= 0.0 && vx >= 0.0 && ux + vx < 1.0);
}

bool IsAnEar(const std::vector<Vector2>& polygons, const size_t prev_index, const size_t index, const size_t next_index)
{
	const auto n = polygons.size();

	const auto prev = polygons[prev_index];
	const auto item = polygons[index % n];
	const auto next = polygons[next_index];

	if (!(Vector2::Cross(item - prev, next - item) > 0.0f))
	{
		return false;
	}

	bool vertexInsideTriangle = false;
	int vertex_count = -1;
	for (const auto& vertex : polygons)
	{
		++vertex_count;
		if (vertex_count == index || vertex_count == prev_index || vertex_count == next_index)
		{
			continue;
		}

		if (IsInsideTriangle(Vector3::ToVector3(prev), Vector3::ToVector3(item), Vector3::ToVector3(next), Vector3::ToVector3(vertex)))
		{
			vertexInsideTriangle = true;
			break;
		}
	}

	return !vertexInsideTriangle;
}

std::vector<Triangle> Triangulation(std::vector<Vector2> polygons)
{
	std::vector<Triangle> triangles;

	std::size_t last_size = 0;
	int same_since = 0;

	for (int i = 0; i < polygons.size();)
	{
		const auto n = polygons.size();

		const auto prev_index = (i - 1 + n) % n;
		const auto next_index = (i + 1) % n;

		if (last_size == polygons.size())
		{
			if (same_since == i)
			{
				break;
			}
		}
		else
		{
			last_size = polygons.size();
			same_since = i;
		}
		if (!IsAnEar(polygons, prev_index, i, next_index))
		{
			++i;
			i = i % polygons.size();
			continue;
		}

		const auto prev = polygons[prev_index];
		const auto item = polygons[i % n];
		const auto next = polygons[next_index];

		triangles.push_back(Triangle{ .prev_point = prev, .point = item, .next_point = next });

		polygons.erase(polygons.begin() + i);
		if (polygons.size() < 3)
		{
			break;
		}
		i = i % polygons.size();
	}

	return triangles;
}

std::vector<Triangle> PolygonColliderComponentInterface::CreatePolygonTriangulation(Entity ent, EntityManager* entman)
{
	const PolygonColliderComponent& polygon_collider = entman->GetComponent<PolygonColliderComponent>(ent);

	std::vector<Vector2> polygons = polygon_collider.points;
	return Triangulation(polygons);
}

void PolygonColliderComponentInterface::SetTrigger(const CSMonoObject& object, bool trigger)
{
	const CSMonoObject game_object = GameObjectInterface::GetGameObjectFromComponent(object);
	const auto scene_index = GameObjectInterface::GetSceneIndex(game_object);
	const auto entity = GameObjectInterface::GetEntityID(game_object);
	PolygonColliderComponent& polygon_collider = SceneManager::GetSceneManager()->GetEntityManager(scene_index)->GetComponent<PolygonColliderComponent>(entity);
	polygon_collider.trigger = trigger;
	polygon_collider.update_polygon_collider = true;
}

void PolygonColliderComponentInterface::SaveScriptComponent(Entity ent, EntityManager* entman, JsonObject* json_object)
{
	const PolygonColliderComponent& polygon_collider = entman->GetComponent<PolygonColliderComponent>(ent);
	json_object->SetData(polygon_collider.trigger, "trigger");
	json_object->SetData(polygon_collider.points.size(), "point_size");
	for (int i = 0; i < polygon_collider.points.size(); ++i)
	{
		json_object->SetData(polygon_collider.points[i], "point_" + std::to_string(i));
	}
	json_object->SetData(polygon_collider.loop, "loop");
	json_object->SetData(polygon_collider.solid, "solid");

	json_object->SetData(polygon_collider.filter.category_bits, "filter.category_bits");
	json_object->SetData(polygon_collider.filter.mask_bits, "filter.mask_bits");
	json_object->SetData(polygon_collider.filter.group_index, "filter.group_index");
}

void PolygonColliderComponentInterface::LoadScriptComponent(Entity ent, EntityManager* entman, JsonObject* json_object)
{
	PolygonColliderComponent& polygon_collider = entman->GetComponent<PolygonColliderComponent>(ent);
	json_object->LoadData(polygon_collider.trigger, "trigger");
	uint64_t point_size = 0;
	json_object->LoadData(point_size, "point_size");
	polygon_collider.points.resize(point_size);

	for (int i = 0; i < polygon_collider.points.size(); ++i)
	{
		json_object->LoadData(polygon_collider.points[i], "point_" + std::to_string(i));
	}
	json_object->LoadData(polygon_collider.loop, "loop");
	json_object->LoadData(polygon_collider.solid, "solid");

	json_object->LoadData(polygon_collider.filter.category_bits, "filter.category_bits");
	json_object->LoadData(polygon_collider.filter.mask_bits, "filter.mask_bits");
	json_object->LoadData(polygon_collider.filter.group_index, "filter.group_index");

	polygon_collider.update_polygon_collider = true;
}
