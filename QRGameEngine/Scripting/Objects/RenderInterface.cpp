#include "pch.h"
#include "RenderInterface.h"
#include "Renderer/RenderCore.h"
#include "Scripting/CSMonoCore.h"
#include "SceneSystem/SceneLoader.h"

MonoClassHandle RenderInterface::texture_handle;
DeferedMethodIndex RenderInterface::s_load_texture_index;

void RenderInterface::RegisterInterface(CSMonoCore* mono_core, const DeferedMethodIndex load_texture_index)
{
    texture_handle = CSMonoCore::Get()->RegisterMonoClass("ScriptProject.Engine", "Texture");

    MonoClassHandle render_handle = CSMonoCore::Get()->RegisterMonoClass("ScriptProject.Engine", "Render");

    MonoMethodHandle load_texture_method_handle = CSMonoCore::Get()->HookAndRegisterMonoMethodType<RenderInterface::LoadTexture>(render_handle, "LoadTexture_External", RenderInterface::LoadTexture);

    s_load_texture_index = load_texture_index;
}

CSMonoObject RenderInterface::LoadTexture(const std::string& texture_name, const SceneIndex scene_index)
{
    CSMonoObject texture_object(CSMonoCore::Get(), texture_handle);
    SceneLoader::Get()->GetDeferedCalls()->TryCallDirectly(scene_index, s_load_texture_index, texture_object, texture_name, scene_index);
    return texture_object;
}

void RenderInterface::LoadAndSetTexture(const CSMonoObject& texture, const std::string& texture_name, SceneIndex scene_index)
{
    TextureHandle handle = RenderCore::Get()->LoadTexture(texture_name, scene_index);
    CSMonoCore::Get()->SetValue(handle, texture, "texture_asset_handle");
}
