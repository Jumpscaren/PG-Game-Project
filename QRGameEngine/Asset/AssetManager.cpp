#include "pch.h"
#include "AssetManager.h"
#include "../Event/EventCore.h"

#pragma warning(push)
#pragma warning(disable:6262)
#pragma warning(disable:26451)
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#pragma warning(pop)

AssetManager* AssetManager::s_asset_manager = nullptr;

AssetManager::AssetManager()
{
	s_asset_manager = this;

	m_asset_loading_thread = std::thread(&AssetManager::ThreadLoadAssetLoop, this);

	EventCore::Get()->ListenToEvent<AssetManager::DeletedSceneEventListener>("DeletedScene", 0, AssetManager::DeletedSceneEventListener);
}

AssetManager::~AssetManager()
{
	{
		std::unique_lock<std::mutex> lock(m_asset_loading_thread_mutex);
		m_terminate_loading_thread = true;
	}
	m_asset_loading_thread_condition_variable.notify_all();
	m_asset_loading_thread.join();
}

AssetManager* AssetManager::Get()
{
	return s_asset_manager;
}

void AssetManager::HandleCompletedJobs()
{
	std::unique_lock<std::mutex> lock(m_assets_access_mutex);
	for (const AssetData& asset : m_asset_loading_thread_completed_jobs)
	{
		bool asset_exists = false;
		const AssetHandle asset_handle = LoadAssetPath(asset.asset_path, asset_exists);
		m_assets.insert({ asset_handle, asset });

		if (!asset_exists)
		{
			DeleteAsset(asset_handle);
			continue;
		}

		EventCore::Get()->SendEvent("Asset Finished Loading", asset_handle);
	}
	m_asset_loading_thread_completed_jobs.clear();
}

AssetHandle AssetManager::LoadTextureAsset(const std::string& texture_path, const SceneIndex scene_index, const AssetLoadFlag& asset_load_flag)
{
	bool texture_exists = false;
	AssetHandle texture_handle = LoadAssetPath(texture_path, texture_exists);

	if (!m_loaded_assets_in_scenes.contains(scene_index))
	{
		qr::unordered_set<AssetHandle> assets;
		m_loaded_assets_in_scenes.insert({ scene_index, assets });
	}

	if (!m_loaded_assets_in_scenes.at(scene_index).contains(texture_handle))
	{
		auto it = m_assets_information.find(texture_handle);
		if (it != m_assets_information.end())
		{
			++it->second.count_asset_loaded_by_scenes;
		}
		else
		{
			m_assets_information.insert({ texture_handle, {.count_asset_loaded_by_scenes = 1} });
		}

		m_loaded_assets_in_scenes.at(scene_index).insert(texture_handle);
	}

	if (!texture_exists)
	{
		AddAssetLoadingJob(AssetLoadingJob{.asset_info_data = AssetData{.asset_data = nullptr, .asset_type = AssetType::TEXTURE, .asset_path = texture_path, .asset_load_flag = asset_load_flag }, .load_function = &AssetManager::LoadTextureData });
	}

	return texture_handle;
}

TextureInfo* AssetManager::GetTextureData(const AssetHandle& texture_handle)
{
	assert(m_assets.contains(texture_handle));

	auto asset_data = m_assets.find(texture_handle);
	if (asset_data->second.asset_type != AssetType::TEXTURE)
	{
		assert(false);
		return nullptr;
	}

	return (TextureInfo*)asset_data->second.asset_data;
}

std::string AssetManager::GetAssetPath(const AssetHandle& asset_handle)
{
	assert(m_assets.contains(asset_handle));

	const auto asset_data = m_assets.find(asset_handle);

	return asset_data->second.asset_path;
}

const AssetManager::AssetLoadFlag& AssetManager::GetAssetLoadFlag(AssetHandle asset_handle)
{
	assert(m_assets.contains(asset_handle));

	const auto asset_data = m_assets.find(asset_handle);

	return asset_data->second.asset_load_flag;
}

void AssetManager::DeleteCPUAssetDataIfGPUOnly(AssetHandle asset_handle)
{
	assert(m_assets.contains(asset_handle));

	const auto asset_data = m_assets.find(asset_handle);
	
	if (asset_data->second.asset_load_flag != AssetLoadFlag::GPU_ONLY)
		return;

	if (asset_data->second.asset_type == AssetType::TEXTURE)
	{
		TextureInfo* texture_info = (TextureInfo*)asset_data->second.asset_data;
		free(texture_info->texture_data);
		texture_info->texture_data = nullptr;
	}
}

bool AssetManager::IsAssetLoaded(AssetHandle asset_handle)
{
	return m_assets.contains(asset_handle);
}

AssetHandle AssetManager::LoadAssetPath(const std::string& asset_path, bool& asset_existing)
{
	uint64_t asset_path_hash = std::hash<std::string>{}(asset_path);

	if (m_loaded_assets.contains(asset_path_hash))
	{
		asset_existing = true;
		return m_loaded_assets.find(asset_path_hash)->second;
	}

	AssetHandle new_asset_handle = m_asset_count++;

	m_loaded_assets.insert({ asset_path_hash, new_asset_handle });

	asset_existing = false;

	return new_asset_handle;
}

void* AssetManager::LoadTextureData(const AssetData& asset_data)
{
	int width, height, comp, channels = 4;
	unsigned char* imageData = stbi_load(asset_data.asset_path.c_str(),
		&width, &height, &comp, channels);
	TextureInfo* texture = new TextureInfo{ imageData, (uint32_t)width, (uint32_t)height, (uint32_t)comp, (uint32_t)channels };

	return texture;
}

void AssetManager::AddAssetLoadingJob(const AssetLoadingJob& job)
{
	{
		std::unique_lock<std::mutex> lock(m_asset_loading_thread_mutex);
		m_asset_loading_thread_jobs.push_back(job);
	}
	m_asset_loading_thread_condition_variable.notify_one();
}

void AssetManager::ThreadLoadAssetLoop()
{
	while (true)
	{
		AssetLoadingJob job;
		{
			std::unique_lock<std::mutex> lock(m_asset_loading_thread_mutex);
			m_asset_loading_thread_condition_variable.wait(lock, [this] {
				return !m_asset_loading_thread_jobs.empty() || m_terminate_loading_thread;
				});

			if (m_terminate_loading_thread)
			{
				return;
			}
			job = m_asset_loading_thread_jobs.back();
			m_asset_loading_thread_jobs.pop_back();
		}

		void* asset_data = job.load_function(job.asset_info_data);
		{
			std::unique_lock<std::mutex> lock(m_assets_access_mutex);
			job.asset_info_data.asset_data = asset_data;
			m_asset_loading_thread_completed_jobs.push_back(job.asset_info_data);
		}
	}
}

void AssetManager::DeleteAsset(const AssetHandle asset)
{
	EventCore::Get()->SendEvent("DeletedAsset", asset);

	auto it = m_assets.find(asset);
	assert(m_assets.contains(asset));

	if (it->second.asset_data && it->second.asset_type == AssetType::TEXTURE)
	{
		TextureInfo* texture_info = (TextureInfo*)it->second.asset_data;
		if (texture_info->texture_data)
		{
			free(texture_info->texture_data);
		}
		delete texture_info;
	}

	uint64_t hashed_path = std::hash<std::string>{}(it->second.asset_path);
	m_loaded_assets.erase(hashed_path);
	m_assets.erase(it);
}

void AssetManager::HandleDeletedScene(const SceneIndex scene_index)
{
	if (auto it = m_loaded_assets_in_scenes.find(scene_index); it != m_loaded_assets_in_scenes.end())
	{
		for (const AssetHandle asset : it->second)
		{
			AssetInformation& asset_information = m_assets_information.at(asset);
			--asset_information.count_asset_loaded_by_scenes;
			if (asset_information.count_asset_loaded_by_scenes == 0)
			{
				DeleteAsset(asset);
				m_assets_information.erase(asset);
			}
		}

		m_loaded_assets_in_scenes.erase(it);
	}
}

void AssetManager::DeletedSceneEventListener(const SceneIndex scene_index)
{
	AssetManager::Get()->HandleDeletedScene(scene_index);
}
