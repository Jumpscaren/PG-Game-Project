#pragma once
#include "Common/DeferMethodCalls.h"
#include <semaphore>
#include "SceneSystem/SceneDefines.h"

class SceneLoaderDeferCalls
{
public:
	SceneLoaderDeferCalls();
	~SceneLoaderDeferCalls() = default;

	template<auto func, typename Type>
		requires (std::is_member_function_pointer_v<decltype(func)>&& DeferMethodCalls::IsSameType<Type>(func))
	DeferedMethodIndex Register(Type* instance)
	{
		return m_defer_calls.Register<func>(instance);
	}

	template<auto func>
	DeferedMethodIndex RegisterStatic()
	{
		return m_defer_calls.RegisterStatic<func>();
	}

	template<typename ...Args>
	void Call(const DeferedMethodIndex index, Args&&... args)
	{
		if (m_defered_calls_count >= MAX_DEFERED_CALLS_BEFORE_FLUSH)
		{
			m_defered_calls_count = 0;
			m_is_full = true;
			m_semaphore.acquire();
		}

		m_defer_calls.Call(index, std::forward<Args>(args)...);

		++m_defered_calls_count;
	}

	template<typename ...Args>
	// If the function is not possible to call directly, then the function is deferred
	bool TryCallDirectly(const SceneIndex current_scene_index, const DeferedMethodIndex defer_index, Args&&... args)
	{
		if (ShouldCallDirectly(current_scene_index))
		{
			m_defer_calls.CallDeferredMethod(defer_index, std::forward<Args>(args)...);
			return true;
		}

		Call(defer_index, std::forward<Args>(args)...);
		return false;
	}

	void ClearBuffers()
	{
		m_defer_calls.ClearBuffers();

		if (IsFull())
		{
			m_semaphore.release();
			m_is_full = false;
		}
	}

	bool IsFull()
	{
		return m_is_full;
	}

private:
	std::size_t m_defered_calls_count = 0;
	static constexpr std::size_t MAX_DEFERED_CALLS_BEFORE_FLUSH = DeferMethodCalls::STOREAGE_MAX_SIZE;
	std::atomic<bool> m_is_full = false;

	DeferMethodCalls m_defer_calls;

	std::binary_semaphore m_semaphore;

private:
	bool ShouldCallDirectly(const SceneIndex current_scene_index) const;
};

