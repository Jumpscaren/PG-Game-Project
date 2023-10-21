#pragma once
#include "Common/EngineTypes.h"

class JsonObject
{
private:
	/*nlohmann::json*/
	void* m_json_object;
	bool m_created_memory = false;

private:
	JsonObject(void* json_object);

public:
	JsonObject();
	JsonObject(const std::string& json_string);
	~JsonObject();

	bool IsObjectDiscarded(const std::string& name);
	bool ObjectExist(const std::string& name);
	JsonObject CreateSubJsonObject(const std::string& name);
	JsonObject GetSubJsonObject(const std::string& name);
	void SetData(uint32_t data, const std::string& name);
	void SetData(uint64_t data, const std::string& name);
	void SetData(float data, const std::string& name);
	void SetData(const Vector2& data, const std::string& name);
	void SetData(char* data, uint32_t data_size, const std::string& name);
	void LoadData(uint32_t& data, const std::string& name);
	void LoadData(uint64_t& data, const std::string& name);
	void LoadData(float& data, const std::string& name);
	void LoadData(Vector2& data, const std::string& name);
	void LoadData(char* data, uint32_t data_size, const std::string& name);

	std::string GetJsonString();
};