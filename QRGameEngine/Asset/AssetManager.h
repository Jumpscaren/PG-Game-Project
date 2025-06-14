#pragma once
#include "Asset/AssetTypes.h"
#include "SceneSystem/SceneDefines.h"

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

	struct AssetInformation
	{
		uint8_t count_asset_loaded_by_scenes;
	};

	struct AssetLoadingJob
	{
		AssetData asset_info_data;
		std::function<void* (const AssetData&)> load_function;
	};

	struct LoadedAssetsInformation
	{
		AssetHandle asset_handle;
		std::string asset_path;
	};

private:
	qr::unordered_map<uint64_t, LoadedAssetsInformation> m_loaded_assets;

	qr::unordered_map<AssetHandle, AssetData> m_assets;

	qr::unordered_map<SceneIndex, qr::unordered_set<AssetHandle>> m_loaded_assets_in_scenes;

	qr::unordered_map<AssetHandle, AssetInformation> m_assets_information;

	static AssetManager* s_asset_manager;

	uint64_t m_asset_count = 0;

	std::thread m_asset_loading_thread;
	std::mutex m_asset_loading_thread_mutex;
	std::condition_variable m_asset_loading_thread_condition_variable;
	std::vector<AssetLoadingJob> m_asset_loading_thread_jobs;
	std::mutex m_assets_access_mutex;
	std::vector<AssetData> m_asset_loading_thread_completed_jobs;
	bool m_terminate_loading_thread = false;

	qr::unordered_set<uint64_t> m_ordered_to_be_removed_assets;

private:
	AssetHandle LoadAssetPath(const std::string& asset_path, bool& asset_existing);

	static void* LoadTextureData(const AssetData& asset_data);
	void DeleteAssetData(void* asset_data, const AssetType asset_type);

	void AddAssetLoadingJob(const AssetLoadingJob& job);
	void ThreadLoadAssetLoop();

	void HandleDeletedScene(SceneIndex scene_index);
	static void DeletedSceneEventListener(SceneIndex scene_index);

public:
	AssetManager();
	~AssetManager();

	static AssetManager* Get();

	void HandleCompletedJobs();

	void ExportTextureDataToPNG(TextureInfo* texture_data, const std::string& texture_name);

	AssetHandle LoadTextureAsset(const std::string& texture_path, SceneIndex scene_index, bool asynchronous = true, const AssetLoadFlag& asset_load_flag = GPU_ONLY);
	TextureInfo* GetTextureData(const AssetHandle& texture_handle);
	std::string GetAssetPath(const AssetHandle& asset_handle);
	const AssetLoadFlag& GetAssetLoadFlag(AssetHandle asset_handle);
	void DeleteCPUAssetDataIfGPUOnly(AssetHandle asset_handle);

	bool IsAssetLoaded(AssetHandle asset_handle);

	void DeleteAsset(AssetHandle asset);
};

