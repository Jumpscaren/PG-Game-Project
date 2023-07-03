#include "pch.h"
#include "AssetManager.h"

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
}

AssetManager::~AssetManager()
{
}

AssetManager* AssetManager::Get()
{
	return s_asset_manager;
}

AssetHandle AssetManager::LoadTexture(const std::string& texture_path)
{
	bool texture_exists = false;
	AssetHandle texture_handle = LoadAssetPath(texture_path, texture_exists);

	if (!texture_exists)
	{
		int width, height, comp, channels = 4;
		unsigned char* imageData = stbi_load(texture_path.c_str(),
			&width, &height, &comp, channels);
		TextureInfo* texture = new TextureInfo{ imageData, (uint32_t)width, (uint32_t)height, (uint32_t)comp, (uint32_t)channels };

		m_assets.insert({ texture_handle, {texture, AssetType::TEXTURE, texture_path} });
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

	auto asset_data = m_assets.find(asset_handle);

	return asset_data->second.asset_path;
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

	m_loaded_assets.insert({asset_path_hash, new_asset_handle });

	asset_existing = false;

	return new_asset_handle;
}
