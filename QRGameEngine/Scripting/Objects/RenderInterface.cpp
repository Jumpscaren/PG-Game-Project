#include "pch.h"
#include "RenderInterface.h"
#include "Renderer/RenderCore.h"
#include "Scripting/CSMonoCore.h"

MonoClassHandle RenderInterface::texture_handle;

void RenderInterface::RegisterInterface(CSMonoCore* mono_core)
{
    texture_handle = CSMonoCore::Get()->RegisterMonoClass("ScriptProject.Engine", "Texture");

    MonoClassHandle render_handle = CSMonoCore::Get()->RegisterMonoClass("ScriptProject.Engine", "Render");

    MonoMethodHandle load_texture_method_handle = CSMonoCore::Get()->HookAndRegisterMonoMethodType<RenderInterface::LoadTexture>(render_handle, "LoadTexture", RenderInterface::LoadTexture);
}

CSMonoObject RenderInterface::LoadTexture(std::string texture_name)
{
    TextureHandle handle = RenderCore::Get()->LoadTexture(texture_name);

    CSMonoObject texture_object(CSMonoCore::Get(), texture_handle);
    CSMonoCore::Get()->SetValue(handle, texture_object, "texture_asset_handle");

    return texture_object;
}
