#include "pch.h"
#include "JsonObject.h"
#include "Vendor/Include/Json/json.hpp"

JsonObject::JsonObject(void* json_object)
{
	m_json_object = json_object;
}

bool JsonObject::IsObjectDiscarded(const std::string& name)
{
	return (*((nlohmann::json*)m_json_object))[name].is_discarded();
}

bool JsonObject::ObjectExist(const std::string& name)
{
	return !(*((nlohmann::json*)m_json_object))[name].is_null();
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
	return (*((nlohmann::json*)m_json_object))[name].is_number_integer();
}

bool JsonObject::IsObjectUnsigned(const std::string& name)
{
	return (*((nlohmann::json*)m_json_object))[name].is_number_unsigned();
}

bool JsonObject::IsObjectFloat(const std::string& name)
{
	return (*((nlohmann::json*)m_json_object))[name].is_number_float();
}

bool JsonObject::IsObjectString(const std::string& name)
{
	return (*((nlohmann::json*)m_json_object))[name].is_string();
}

bool JsonObject::IsObjectBool(const std::string& name)
{
	return (*((nlohmann::json*)m_json_object))[name].is_boolean();
}

bool JsonObject::IsObject(const std::string& name)
{
	return (*((nlohmann::json*)m_json_object))[name].is_object();
}

JsonObject JsonObject::CreateSubJsonObject(const std::string& name)
{
	(*((nlohmann::json*)m_json_object))[name] = {};
	nlohmann::json* json = &(*((nlohmann::json*)m_json_object))[name];
	return JsonObject((void*)json);
}

JsonObject JsonObject::GetSubJsonObject(const std::string& name)
{
	nlohmann::json* json = &(*((nlohmann::json*)m_json_object))[name];
	return JsonObject((void*)json);
}

void JsonObject::SetData(uint8_t data, const std::string& name)
{
	(*((nlohmann::json*)m_json_object))[name] = data;
}

void JsonObject::SetData(uint16_t data, const std::string& name)
{
	(*((nlohmann::json*)m_json_object))[name] = data;
}

void JsonObject::SetData(int16_t data, const std::string& name)
{
	(*((nlohmann::json*)m_json_object))[name] = data;
}

void JsonObject::SetData(uint32_t data, const std::string& name)
{
	(*((nlohmann::json*)m_json_object))[name] = data;
}

void JsonObject::SetData(int32_t data, const std::string& name)
{
	(*((nlohmann::json*)m_json_object))[name] = data;
}

void JsonObject::SetData(uint64_t data, const std::string& name)
{
	(*((nlohmann::json*)m_json_object))[name] = data;
}

void JsonObject::SetData(float data, const std::string& name)
{
	(*((nlohmann::json*)m_json_object))[name] = data;
}

void JsonObject::SetData(double data, const std::string& name)
{
	(*((nlohmann::json*)m_json_object))[name] = data;
}

void JsonObject::SetData(bool data, const std::string& name)
{
	(*((nlohmann::json*)m_json_object))[name] = data;
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

void JsonObject::SetData(char* data, uint32_t data_size, const std::string& name)
{
	std::vector<uint8_t> v(data, data + data_size);
	(*((nlohmann::json*)m_json_object))[name] = v;
}

void JsonObject::SetData(const std::string& data, const std::string& name)
{
	(*((nlohmann::json*)m_json_object))[name] = data;
}

void JsonObject::LoadData(uint8_t& data, const std::string& name)
{
	if (!ObjectExist(name))
	{
		data = 0;
		return;
	}
	data = (*((nlohmann::json*)m_json_object))[name];
}

void JsonObject::LoadData(uint16_t& data, const std::string& name)
{
	if (!ObjectExist(name))
	{
		data = 0;
		return;
	}
	data = (*((nlohmann::json*)m_json_object))[name];
}

void JsonObject::LoadData(int16_t& data, const std::string& name)
{
	if (!ObjectExist(name))
	{
		data = 0;
		return;
	}
	data = (*((nlohmann::json*)m_json_object))[name];
}

void JsonObject::LoadData(uint32_t& data, const std::string& name)
{
	if (!ObjectExist(name))
	{
		data = 0;
		return;
	}
	data = (*((nlohmann::json*)m_json_object))[name];
}

void JsonObject::LoadData(int32_t& data, const std::string& name)
{
	if (!ObjectExist(name))
	{
		data = 0;
		return;
	}
	data = (*((nlohmann::json*)m_json_object))[name];
}

void JsonObject::LoadData(uint64_t& data, const std::string& name)
{
	if (!ObjectExist(name))
	{
		data = 0;
		return;
	}
	data = (*((nlohmann::json*)m_json_object))[name];
}

void JsonObject::LoadData(float& data, const std::string& name)
{
	if (!ObjectExist(name))
	{
		data = 0.0f;
		return;
	}
	data = (*((nlohmann::json*)m_json_object))[name];
}

void JsonObject::LoadData(double& data, const std::string& name)
{
	if (!ObjectExist(name))
	{
		data = 0.0;
		return;
	}
	data = (*((nlohmann::json*)m_json_object))[name];
}

void JsonObject::LoadData(bool& data, const std::string& name)
{
	if (!ObjectExist(name))
	{
		data = false;
		return;
	}
	data = (*((nlohmann::json*)m_json_object))[name];
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

void JsonObject::LoadData(char* data, uint32_t data_size, const std::string& name)
{
	if (!ObjectExist(name))
	{
		memset(data, 0, data_size);
		return;
	}
	std::vector<uint8_t> v = (*((nlohmann::json*)m_json_object))[name];
	memcpy(data, v.data(), data_size);
}

void JsonObject::LoadData(std::string& data, const std::string& name)
{
	if (!ObjectExist(name))
	{
		data = "";
		return;
	}
	data = (*((nlohmann::json*)m_json_object))[name];
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
