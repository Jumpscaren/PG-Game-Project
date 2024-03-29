#pragma once
#include "Asset/AssetTypes.h"

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

private:
	std::unordered_map<uint64_t, AssetHandle> m_loaded_assets;

	std::unordered_map<AssetHandle, AssetData> m_assets;

	static AssetManager* s_asset_manager;

	uint64_t m_asset_count = 0;

private:
	AssetHandle LoadAssetPath(const std::string& asset_path, bool& asset_existing);

public:
	AssetManager();
	~AssetManager();

	static AssetManager* Get();

	AssetHandle LoadTextureAsset(const std::string& texture_path, const AssetLoadFlag& asset_load_flag = GPU_ONLY);
	TextureInfo* GetTextureData(const AssetHandle& texture_handle);
	std::string GetAssetPath(const AssetHandle& asset_handle);
	const AssetLoadFlag& GetAssetLoadFlag(AssetHandle asset_handle);
	void DeleteCPUAssetDataIfGPUOnly(AssetHandle asset_handle);
};

