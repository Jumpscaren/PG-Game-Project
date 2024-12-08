#pragma once
#include "Asset/AssetTypes.h"
#include <thread>
#include <mutex>
#include <functional>

struct TextureInfo
{
	unsigned char* texture_data;
	uint32_t width, height, comp, channels;
};

class AssetManager
{
public:
	enum AssetLoadFlag
	{
		CPU_ONLY,
		GPU_ONLY,
		CPU_AND_GPU
	};

private:
	enum AssetType
	{
		TEXTURE = 0,
	};

	struct AssetData
	{
		void* asset_data;
		AssetType asset_type;
		std::string asset_path;
		AssetLoadFlag asset_load_flag;
	};

	struct AssetLoadingJob
	{
		AssetData asset_info_data;
		std::function<void* (const AssetData&)> load_function;
	};

private:
	qr::unordered_map<uint64_t, AssetHandle> m_loaded_assets;

	qr::unordered_map<AssetHandle, AssetData> m_assets;

	static AssetManager* s_asset_manager;

	uint64_t m_asset_count = 0;

	std::thread m_asset_loading_thread;
	std::mutex m_asset_loading_thread_mutex;
	std::condition_variable m_asset_loading_thread_condition_variable;
	std::vector<AssetLoadingJob> m_asset_loading_thread_jobs;
	std::mutex m_assets_access_mutex;
	std::vector<AssetData> m_asset_loading_thread_completed_jobs;
	bool m_terminate_loading_thread = false;

private:
	AssetHandle LoadAssetPath(const std::string& asset_path, bool& asset_existing);

	static void* LoadTextureData(const AssetData& asset_data);

	void AddAssetLoadingJob(const AssetLoadingJob& job);
	void ThreadLoadAssetLoop();

public:
	AssetManager();
	~AssetManager();

	static AssetManager* Get();

	void HandleCompletedJobs();

	AssetHandle LoadTextureAsset(const std::string& texture_path, const AssetLoadFlag& asset_load_flag = GPU_ONLY);
	TextureInfo* GetTextureData(const AssetHandle& texture_handle);
	std::string GetAssetPath(const AssetHandle& asset_handle);
	const AssetLoadFlag& GetAssetLoadFlag(AssetHandle asset_handle);
	void DeleteCPUAssetDataIfGPUOnly(AssetHandle asset_handle);

	bool IsAssetLoaded(AssetHandle asset_handle);
};

