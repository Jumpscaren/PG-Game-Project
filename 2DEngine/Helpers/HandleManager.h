#pragma once
class HandleManager
{
public:
	enum class HandleType
	{
		TEXTURE,
		TEXTURE_VIEW,
		BUFFER,
		BUFFER_VIEW,
	};

private:
	std::unordered_map<HandleType, std::vector<uint64_t>> m_handles;
	static HandleManager s_handle_manager;

private:
	HandleManager();

public:
	~HandleManager();
	HandleManager(const HandleManager& other) = delete;
	HandleManager& operator=(const HandleManager& other) = delete;

	static bool GetHandle(const HandleType& handle_type, uint64_t& handle);
	static void FreeHandle(const HandleType& handle_type, uint64_t handle);
};

