#include "pch.h"
#include "TextureInterface.h"
#include "Scripting/CSMonoCore.h"

MonoClassHandle TextureInterface::s_texture_object_class;

void TextureInterface::RegisterInterface(CSMonoCore* mono_core)
{
    s_texture_object_class = mono_core->RegisterMonoClass("ScriptProject.Engine", "Texture");
}

CSMonoObject TextureInterface::CreateTexture(TextureHandle texture_handle)
{
    CSMonoObject cs_texture(CSMonoCore::Get(), s_texture_object_class);

    CSMonoCore::Get()->SetValue(texture_handle, cs_texture, "texture_asset_handle");

    return cs_texture;
}
