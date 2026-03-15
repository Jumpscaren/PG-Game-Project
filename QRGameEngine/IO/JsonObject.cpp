#include "pch.h"
#include "JsonObject.h"
#include "Vendor/Include/Json/json.hpp"

namespace
{
	nlohmann::json& getJSONType(void* json_object)
	{
		return *((nlohmann::json*)json_object);
	}
}

JsonObject::JsonObject(void* json_object)
{
	m_json_object = json_object;
}

bool JsonObject::IsObjectDiscarded(const std::string& name)
{
	return getJSONType(m_json_object)[name].is_discarded();
}

bool JsonObject::ObjectExist(const std::string& name)
{
	return !getJSONType(m_json_object)[name].is_null();
}

JsonObject::JsonObject()
{
	m_json_object = new nlohmann::json();
	m_created_memory = true;
}

JsonObject::JsonObject(const std::string& json_string)
{
	m_json_object = new nlohmann::json();
	*((nlohmann::json*)m_json_object) = nlohmann::json::parse(json_string);
	m_created_memory = true;
}

JsonObject::~JsonObject()
{
	if (m_created_memory)
		delete (nlohmann::json*)(m_json_object);
}

bool JsonObject::IsObjectInteger(const std::string& name)
{
	return getJSONType(m_json_object)[name].is_number_integer();
}

bool JsonObject::IsObjectUnsigned(const std::string& name)
{
	return getJSONType(m_json_object)[name].is_number_unsigned();
}

bool JsonObject::IsObjectFloat(const std::string& name)
{
	return getJSONType(m_json_object)[name].is_number_float();
}

bool JsonObject::IsObjectString(const std::string& name)
{
	return getJSONType(m_json_object)[name].is_string();
}

bool JsonObject::IsObjectBool(const std::string& name)
{
	return getJSONType(m_json_object)[name].is_boolean();
}

bool JsonObject::IsObjectVector3(const std::string& name)
{
	if (!IsObject(name))
	{
		return false;
	}

	const bool has_three_members = getJSONType(m_json_object)[name].size() == 3;
	if (!has_three_members)
	{
		return false;
	}

	JsonObject object = GetSubJsonObject(name);
	const bool x_value_exists = object.ObjectExist("x") && object.IsObjectFloat("x");
	const bool y_value_exists = object.ObjectExist("y") && object.IsObjectFloat("y");
	const bool z_value_exists = object.ObjectExist("z") && object.IsObjectFloat("z");
	return x_value_exists && y_value_exists && z_value_exists;
}

bool JsonObject::IsObjectVector4(const std::string& name)
{
	if (!IsObject(name))
	{
		return false;
	}

	const bool has_four_members = getJSONType(m_json_object)[name].size() == 4;
	if (!has_four_members)
	{
		return false;
	}

	JsonObject object = GetSubJsonObject(name);
	const bool x_value_exists = object.ObjectExist("x") && object.IsObjectFloat("x");
	const bool y_value_exists = object.ObjectExist("y") && object.IsObjectFloat("y");
	const bool z_value_exists = object.ObjectExist("z") && object.IsObjectFloat("z");
	const bool w_value_exists = object.ObjectExist("w") && object.IsObjectFloat("w");
	return x_value_exists && y_value_exists && z_value_exists && w_value_exists;
}

bool JsonObject::IsObject(const std::string& name)
{
	return getJSONType(m_json_object)[name].is_object();
}

JsonObject JsonObject::CreateSubJsonObject(const std::string& name)
{
	getJSONType(m_json_object)[name] = {};
	nlohmann::json* json = &(getJSONType(m_json_object))[name];
	return JsonObject((void*)json);
}

JsonObject JsonObject::GetSubJsonObject(const std::string& name)
{
	nlohmann::json* json = &(getJSONType(m_json_object))[name];
	return JsonObject((void*)json);
}

void JsonObject::SetData(uint8_t data, const std::string& name)
{
	getJSONType(m_json_object)[name] = data;
}

void JsonObject::SetData(uint16_t data, const std::string& name)
{
	getJSONType(m_json_object)[name] = data;
}

void JsonObject::SetData(int16_t data, const std::string& name)
{
	getJSONType(m_json_object)[name] = data;
}

void JsonObject::SetData(uint32_t data, const std::string& name)
{
	getJSONType(m_json_object)[name] = data;
}

void JsonObject::SetData(int32_t data, const std::string& name)
{
	getJSONType(m_json_object)[name] = data;
}

void JsonObject::SetData(uint64_t data, const std::string& name)
{
	getJSONType(m_json_object)[name] = data;
}

void JsonObject::SetData(float data, const std::string& name)
{
	getJSONType(m_json_object)[name] = data;
}

void JsonObject::SetData(double data, const std::string& name)
{
	getJSONType(m_json_object)[name] = data;
}

void JsonObject::SetData(bool data, const std::string& name)
{
	getJSONType(m_json_object)[name] = data;
}

void JsonObject::SetData(const Vector2& data, const std::string& name)
{
	JsonObject vector = CreateSubJsonObject(name);
	vector.SetData(data.x, "x");
	vector.SetData(data.y, "y");
}

void JsonObject::SetData(const Vector3& data, const std::string& name)
{
	JsonObject vector = CreateSubJsonObject(name);
	vector.SetData(data.x, "x");
	vector.SetData(data.y, "y");
	vector.SetData(data.z, "z");
}

void JsonObject::SetData(const Vector4& data, const std::string& name)
{
	JsonObject vector = CreateSubJsonObject(name);
	vector.SetData(data.x, "x");
	vector.SetData(data.y, "y");
	vector.SetData(data.z, "z");
	vector.SetData(data.w, "w");
}

void JsonObject::SetData(char* data, uint32_t data_size, const std::string& name)
{
	std::vector<uint8_t> v(data, data + data_size);
	getJSONType(m_json_object)[name] = v;
}

void JsonObject::SetData(const std::string& data, const std::string& name)
{
	getJSONType(m_json_object)[name] = data;
}

void JsonObject::LoadData(uint8_t& data, const std::string& name)
{
	if (!ObjectExist(name))
	{
		data = 0;
		return;
	}
	data = getJSONType(m_json_object)[name];
}

void JsonObject::LoadData(uint16_t& data, const std::string& name)
{
	if (!ObjectExist(name))
	{
		data = 0;
		return;
	}
	data = getJSONType(m_json_object)[name];
}

void JsonObject::LoadData(int16_t& data, const std::string& name)
{
	if (!ObjectExist(name))
	{
		data = 0;
		return;
	}
	data = getJSONType(m_json_object)[name];
}

void JsonObject::LoadData(uint32_t& data, const std::string& name)
{
	if (!ObjectExist(name))
	{
		data = 0;
		return;
	}
	data = getJSONType(m_json_object)[name];
}

void JsonObject::LoadData(int32_t& data, const std::string& name)
{
	if (!ObjectExist(name))
	{
		data = 0;
		return;
	}
	data = getJSONType(m_json_object)[name];
}

void JsonObject::LoadData(uint64_t& data, const std::string& name)
{
	if (!ObjectExist(name))
	{
		data = 0;
		return;
	}
	data = getJSONType(m_json_object)[name];
}

void JsonObject::LoadData(float& data, const std::string& name)
{
	if (!ObjectExist(name))
	{
		data = 0.0f;
		return;
	}
	data = getJSONType(m_json_object)[name];
}

void JsonObject::LoadData(double& data, const std::string& name)
{
	if (!ObjectExist(name))
	{
		data = 0.0;
		return;
	}
	data = getJSONType(m_json_object)[name];
}

void JsonObject::LoadData(bool& data, const std::string& name)
{
	if (!ObjectExist(name))
	{
		data = false;
		return;
	}
	data = getJSONType(m_json_object)[name];
}

void JsonObject::LoadData(Vector2& data, const std::string& name)
{
	if (!ObjectExist(name))
	{
		data.x = 0.0f;
		data.y = 0.0f;
		return;
	}
	JsonObject vector = GetSubJsonObject(name);
	vector.LoadData(data.x, "x");
	vector.LoadData(data.y, "y");
}

void JsonObject::LoadData(Vector3& data, const std::string& name)
{
	if (!ObjectExist(name))
	{
		data.x = 0.0f;
		data.y = 0.0f;
		data.z = 0.0f;
		return;
	}
	JsonObject vector = GetSubJsonObject(name);
	vector.LoadData(data.x, "x");
	vector.LoadData(data.y, "y");
	vector.LoadData(data.z, "z");
}

void JsonObject::LoadData(Vector4& data, const std::string& name)
{
	if (!ObjectExist(name))
	{
		data.x = 0.0f;
		data.y = 0.0f;
		data.z = 0.0f;
		data.w = 0.0f;
		return;
	}
	JsonObject vector = GetSubJsonObject(name);
	vector.LoadData(data.x, "x");
	vector.LoadData(data.y, "y");
	vector.LoadData(data.z, "z");
	vector.LoadData(data.w, "w");
}

void JsonObject::LoadData(char* data, uint32_t data_size, const std::string& name)
{
	if (!ObjectExist(name))
	{
		memset(data, 0, data_size);
		return;
	}
	std::vector<uint8_t> v = getJSONType(m_json_object)[name];
	memcpy(data, v.data(), data_size);
}

void JsonObject::LoadData(std::string& data, const std::string& name)
{
	if (!ObjectExist(name))
	{
		data = "";
		return;
	}
	data = getJSONType(m_json_object)[name];
}

std::vector<std::string> JsonObject::GetObjectNames()
{
	std::vector<std::string> names;
	for (auto& el : (*((nlohmann::json*)m_json_object)).items()) {
		names.push_back(el.key());
	}
	return names;
}

std::string JsonObject::GetJsonString()
{
	return ((nlohmann::json*)m_json_object)->dump();
}
