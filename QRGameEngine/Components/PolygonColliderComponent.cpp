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

void PolygonColliderComponentInterface::RegisterInterface(CSMonoCore* mono_core)
{
	auto polygon_collider_class = mono_core->RegisterMonoClass("ScriptProject.Engine", "PolygonCollider");

	mono_core->HookAndRegisterMonoMethodType<PolygonColliderComponentInterface::InitComponent>(polygon_collider_class, "InitComponent", PolygonColliderComponentInterface::InitComponent);
	mono_core->HookAndRegisterMonoMethodType<PolygonColliderComponentInterface::HasComponent>(polygon_collider_class, "HasComponent", PolygonColliderComponentInterface::HasComponent);
	mono_core->HookAndRegisterMonoMethodType<PolygonColliderComponentInterface::RemoveComponent>(polygon_collider_class, "RemoveComponent", PolygonColliderComponentInterface::RemoveComponent);

	mono_core->HookAndRegisterMonoMethodType<PolygonColliderComponentInterface::SetTrigger>(polygon_collider_class, "SetTrigger", PolygonColliderComponentInterface::SetTrigger);

	SceneLoader::Get()->OverrideSaveComponentMethod<PolygonColliderComponent>(SaveScriptComponent, LoadScriptComponent);
}

void PolygonColliderComponentInterface::InitComponent(const CSMonoObject& object, SceneIndex scene_index, Entity entity)
{
	EntityManager* entity_manager = SceneManager::GetSceneManager()->GetEntityManager(scene_index);

	//So that we do not need add staticbody when adding a polygon collider if we do not use a dynamic body
	if (!entity_manager->HasComponent<DynamicBodyComponent>(entity) && !entity_manager->HasComponent<StaticBodyComponent>(entity) && !entity_manager->HasComponent<PureStaticBodyComponent>(entity))
	{
		PhysicsCore::Get()->AddPhysicObject(scene_index, entity, PhysicsCore::StaticBody);
	}

	PhysicsCore::Get()->AddPolygonCollider(scene_index, entity, { Vector2(0.5f, 0.5f), Vector2(-0.5f, -0.5f), Vector2(1.0f, 0.0f) });
}

bool PolygonColliderComponentInterface::HasComponent(const CSMonoObject& object, SceneIndex scene_index, Entity entity)
{
	return SceneManager::GetSceneManager()->GetEntityManager(scene_index)->HasComponent<PolygonColliderComponent>(entity);
}

void PolygonColliderComponentInterface::RemoveComponent(const CSMonoObject& object, SceneIndex scene_index, Entity entity)
{
	PhysicsCore::Get()->RemovePolygonCollider(scene_index, entity);
}

enum Turn
{
	Right,
	Left,
	NoTurn
} turn;

Turn GetTurn(const Vector3& p, const Vector3& u, const Vector3& n, const Vector3& q)
{
	const auto v = Vector3::Cross(q - p, u);
	const auto d = Vector3::Dot(v, n);

	const auto epsilon = 1e-6;
	if (d > epsilon)
	{
		turn = Right;
	}
	if (d < -epsilon)
	{
		turn = Left;
	}
	return NoTurn;
}

bool isInsideTriangle(const Vector3& a, const Vector3& b, const Vector3& c, const Vector3& q)
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

bool IsEar(const std::vector<Vector2>& polygons, int index, const Vector3& normal)
{
	SceneManager* scene_manager = SceneManager::GetSceneManager();
	EntityManager* entity_manager = scene_manager->GetEntityManager(scene_manager->GetActiveSceneIndex());

	const auto n = polygons.size();
	if (n < 3)
	{
		return false;
	}
	if (n == 3)
	{
		return true;
	}

	const auto prev_index = (index - 1 + n) % n;
	const auto next_index = (index + 1) % n;

	const auto prev = polygons[prev_index];
	const auto item = polygons[index];
	const auto next = polygons[next_index];

	const Vector3 p1 = Vector3(prev.x, prev.y, 0.0f);
	const Vector3 i2 = Vector3(item.x, item.y, 0.0f);
	const Vector3 n3 = Vector3(next.x, next.y, 0.0f);

	const auto u = (i2 - p1).Normalize();

	if (GetTurn(p1, u, normal, n3) != Turn::Right)
	{
		return false;
	}

	for (int j = 0; j < n; ++j)
	{
		if (j == index || j == prev_index || j == next_index)
		{
			continue;
		}

		const auto polygon_vertex = polygons[j];
		const Vector3 poly4 = Vector3(polygon_vertex.x, polygon_vertex.y);

		const auto inside = isInsideTriangle(p1, i2, n3, poly4);

		if (inside)
		{
			return false;
		}
	}
	return true;
}

int GetBiggestEar(const std::vector<Vector2>& polygons, const Vector3& normal)
{
	const auto n = polygons.size();
	if (n == 3)
	{
		return 0;
	}
	if (n == 0)
	{
		return -1;
	}

	int max_index = -1;
	float max_area = -10000000000000000000000.0f;

	for (int i = 0; i < n; ++i)
	{
		if (IsEar(polygons, i, normal))
		{
			const auto prev_index = (i - 1 + n) % n;
			const auto next_index = (i + 1) % n;

			const auto prev = polygons[prev_index];
			const auto item = polygons[i];
			const auto next = polygons[next_index];

			const Vector3 p1 = Vector3(prev.x, prev.y, 0.0f);
			const Vector3 i2 = Vector3(item.x, item.y, 0.0f);
			const Vector3 n3 = Vector3(next.x, next.y, 0.0f);

			const auto c = Vector3::Cross(i2 - p1, n3 - p1);
			const auto area = c.Length() * c.Length() / 4.0f;

			if (area > max_area)
			{
				max_index = i;
				max_area = area;
			}
		}
	}

	return max_index;
}

int GetOverlappingEar(const std::vector<Vector2>& polygons, const Vector3& normal)
{
	const auto n = polygons.size();
	if (n == 0)
	{
		return -1;
	}
	if (n == 3)
	{
		return 0;
	}

	for (int k = 0; k < n; ++k)
	{
		const auto prev_index = (k - 1 + n) % n;
		const auto next_index = (k + 1) % n;

		const auto prev = polygons[prev_index];
		const auto item = polygons[k];
		const auto next = polygons[next_index];

		const Vector3 p1 = Vector3(prev.x, prev.y, 0.0f);
		const Vector3 i2 = Vector3(item.x, item.y, 0.0f);
		const Vector3 n3 = Vector3(next.x, next.y, 0.0f);

		const auto u = (i2 - p1).Normalize();

		if (GetTurn(p1, u, normal, n3) != Turn::NoTurn)
		{
			continue;
		}

		const auto v = (n3 - i2).Normalize();

		if (Vector3::Dot(u, v) < 0.0f)
		{
			return k;
		}
	}

	return -1;
}

std::vector<Triangle> CutTriangulation(std::vector<Vector2> polygons, const Vector3& normal)
{
	std::vector<Triangle> triangles;

	for (int i = 0; i < polygons.size();)
	{
		const auto n = polygons.size();

		auto index = GetBiggestEar(polygons, normal);

		if (index == -1)
		{
			index = GetOverlappingEar(polygons, normal);
		}

		if (index == -1)
		{
			index = 0;
		}

		const auto prev_index = (index - 1 + n) % n;
		const auto next_index = (index + 1) % n;

		const auto prev = polygons[prev_index];
		const auto item = polygons[index % n];
		const auto next = polygons[next_index];

		triangles.push_back(Triangle{ .prev_point = prev, .point = item, .next_point = next });

		polygons.erase(polygons.begin() + index);
		if (index <= i)
		{
			--i;
		}
		if (polygons.size() < 3)
		{
			break;
		}
		++i;
		if (i >= 0)
		{
			i = i % polygons.size();
		}
	}

	return triangles;
}

//https://github.com/StefanJohnsen/pyTriangulate/tree/main
std::vector<Triangle> PolygonColliderComponentInterface::CreatePolygonTriangulation(Entity ent, EntityManager* entman)
{
	const PolygonColliderComponent& polygon_collider = entman->GetComponent<PolygonColliderComponent>(ent);

	Vector3 normal;
	for (int i = 0; i < polygon_collider.points.size(); ++i)
	{
		const auto item = polygon_collider.points[i];
		const auto next = polygon_collider.points[(i + 1) % polygon_collider.points.size()];

		const Vector3 v1 = Vector3(item.x, item.y, 0.0f);
		const Vector3 v2 = Vector3(next.x, next.y, 0.0f);

		normal.x += (v2.y - v1.y) * (v2.z + v1.z);
		normal.y += (v2.z - v1.z) * (v2.x + v1.x);
		normal.z += (v2.x - v1.x) * (v2.y + v1.y);
	}
	normal = normal.Normalize();

	std::vector<Vector2> polygons = polygon_collider.points;
	std::ranges::reverse(polygons);

	return CutTriangulation(polygons, normal);
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
}

void PolygonColliderComponentInterface::LoadScriptComponent(Entity ent, EntityManager* entman, JsonObject* json_object)
{
	PolygonColliderComponent& polygon_collider = entman->GetComponent<PolygonColliderComponent>(ent);
	Vector2 half_box_size;
	json_object->LoadData(polygon_collider.trigger, "trigger");
	uint64_t point_size = 0;
	json_object->LoadData(point_size, "point_size");
	polygon_collider.points.resize(point_size);

	for (int i = 0; i < polygon_collider.points.size(); ++i)
	{
		json_object->LoadData(polygon_collider.points[i], "point_" + std::to_string(i));
	}

	polygon_collider.update_polygon_collider = true;
}
