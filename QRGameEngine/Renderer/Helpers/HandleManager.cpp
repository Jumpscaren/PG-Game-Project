#include "pch.h"
#include "HandleManager.h"

HandleManager* HandleManager::s_handle_manager = nullptr;

HandleManager::HandleManager()
{
	s_handle_manager = this;
}

HandleManager::~HandleManager()
{
	s_handle_manager->m_handles.clear();
}

bool HandleManager::GetHandle(const HandleType& handle_type, uint64_t& handle)
{
	auto it = s_handle_manager->m_handles.find(handle_type);
	if (it == s_handle_manager->m_handles.end())
	{
		s_handle_manager->m_handles.insert({ handle_type, {} });
		return false;
	}

	if (it->second.size() == 0)
		return false;
	handle = it->second.back();
	it->second.pop_back();
	return true;
}

void HandleManager::FreeHandle(const HandleType& handle_type, uint64_t handle)
{
	auto it = s_handle_manager->m_handles.find(handle_type);

	if (it == s_handle_manager->m_handles.end())
	{
		assert(false);
		return;
	}

	it->second.push_back(handle);
}
