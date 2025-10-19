#pragma once
#include "DeferMethodCallsDefine.h"

class DeferMethodCalls
{
public:
	static constexpr std::size_t STOREAGE_MAX_SIZE = 100;

	template<typename InstanceType, typename Type, typename ...Args>
	constexpr static bool IsSameType(void(Type::*)(Args...))
	{
		return std::is_same_v<InstanceType, Type>;
	}

	template <typename ...Args>
	using DecayTuple = std::tuple<std::decay_t<Args>...>;

public:
	template<auto func, typename Type>
		requires (std::is_member_function_pointer_v<decltype(func)>&& IsSameType<Type>(func))
	DeferedMethodIndex Register(Type* instance)
	{
		DeferedMethodData defered_function{};
		defered_function.hooked_function = InternalHook<func>(func);
		defered_function.hooked_direct_function = InternalHookDirect<func>(func);
		defered_function.instance = instance;
		defered_function.clear_buffer_function = HookClearBuffer(func);
		defered_function.storage = AllocateStorage(func);
		defered_function.argument_list_info = CreateArgumentList(func);

		m_defered_methods.push_back(defered_function);
		return m_defered_methods.size() - 1;
	}

	template<auto func>
	DeferedMethodIndex RegisterStatic()
	{
		DeferedMethodData defered_function{};
		defered_function.hooked_function = InternalHook<func>(func);
		defered_function.hooked_direct_function = InternalHookDirect<func>(func);
		defered_function.instance = nullptr;
		defered_function.clear_buffer_function = HookClearBuffer(func);
		defered_function.storage = AllocateStorage(func);
		defered_function.argument_list_info = CreateArgumentList(func);

		m_defered_methods.push_back(defered_function);
		return m_defered_methods.size() - 1;
	}

	template<typename ...Args>
	void Call(const DeferedMethodIndex index, Args&&... args)
	{
		if (m_defered_methods.size() <= index)
		{
			assert(false);
		}

		DeferedMethodData& defered_method = m_defered_methods.at(index);

		assert(ArgumentListExist<Args...>(defered_method.argument_list_info));

		char* moved_storage_ptr = defered_method.storage + sizeof(DecayTuple<Args...>) * defered_method.current_storage_index++;
		new (moved_storage_ptr)DecayTuple<Args...>(std::forward<Args>(args)...);

		DecayTuple<Args...>* tuple = (DecayTuple<Args...>*)(moved_storage_ptr);
	}

	template<typename ...Args>
	void CallDeferredMethod(const DeferedMethodIndex index, Args&&... args)
	{
		if (m_defered_methods.size() <= index)
		{
			assert(false);
		}

		DeferedMethodData& defered_method = m_defered_methods.at(index);

		assert(ArgumentListExist<Args...>(defered_method.argument_list_info));

		((void(*)(void*, Args&&...))defered_method.hooked_direct_function)(defered_method.instance, std::forward<Args>(args)...);
	}

	void ClearBuffers()
	{
		for (DeferedMethodData& defered_method : m_defered_methods)
		{
			((void(*)(const DeferedMethodData&))defered_method.clear_buffer_function)(defered_method);
			defered_method.current_storage_index = 0;
		}
	}

private:

	struct ArgumentListInfo
	{
		std::size_t local_index = 0;
		std::size_t global_index = 0;
	};

	struct DeferedMethodData {
		void* hooked_function = nullptr;
		void* hooked_direct_function = nullptr;
		void* instance = nullptr;
		void* clear_buffer_function = nullptr;

		char* storage = nullptr;
		ArgumentListInfo argument_list_info;

		std::size_t current_storage_index = 0;
	};

	static std::size_t GetNewGlobalArgumentListIndex()
	{
		static std::size_t global_argument_list_index = 0;
		return global_argument_list_index++;
	}

	template<typename ...Args>
	static std::vector<std::size_t>& GetArgumentListStoreage()
	{
		static std::vector<std::size_t> argument_list_storage;
		return argument_list_storage;
	}

	template<typename Type, typename ...Args>
	static ArgumentListInfo CreateArgumentList(void(Type::*)(Args...))
	{
		std::vector<std::size_t>& argument_list_storage = GetArgumentListStoreage<std::decay_t<Args>...>();
		argument_list_storage.push_back(GetNewGlobalArgumentListIndex());
		return ArgumentListInfo{ .local_index = argument_list_storage.size() - 1, .global_index = argument_list_storage.back() };
	}

	template<typename ...Args>
	static ArgumentListInfo CreateArgumentList(void(*)(Args...))
	{
		std::vector<std::size_t>& argument_list_storage = GetArgumentListStoreage<std::decay_t<Args>...>();
		argument_list_storage.push_back(GetNewGlobalArgumentListIndex());
		return ArgumentListInfo{ .local_index = argument_list_storage.size() - 1, .global_index = argument_list_storage.back() };
	}

	template<typename ...Args>
	static bool ArgumentListExist(const ArgumentListInfo& argument_list_info)
	{
		const std::vector<std::size_t>& argument_list_storage = GetArgumentListStoreage<std::decay_t<Args>...>();

		if (argument_list_storage.size() <= argument_list_info.local_index)
		{
			return true;
		}

		return argument_list_storage[argument_list_info.local_index] == argument_list_info.global_index;
	}

	template<auto func, typename Type, typename ...Args>
	static void DirectMethodCaller(void* instance, Args&&... args)
	{
		(((Type*)instance)->*func)(std::forward<Args>(args)...);
	}

	template<auto func, typename ...Args>
	static void DirectMethodCallerStatic(void*, Args&&... args)
	{
		(func)(std::forward<Args>(args)...);
	}

	template<auto func, typename Type, typename ...Args>
	static void DeferedMethodCaller(void* instance, Args... args)
	{
		(((Type*)instance)->*func)(args...);
	}

	template<auto func, typename ...Args>
	static void DeferedMethodCallerStatic(void*, Args... args)
	{
		(func)(args...);
	}

	template <typename MethodType, typename Tuple, std::size_t ...I>
	static void PassThrough(void* instance, MethodType method, const Tuple& tuple, const std::index_sequence<I...>&)
	{
		(method)(instance, std::get<I>(tuple)...);
	}

	template <typename ...Args>
	static void ClearDeferedMethodBuffer(const DeferMethodCalls::DeferedMethodData& defered_method)
	{
		constexpr std::size_t STORAGE_SIZE = sizeof(DecayTuple<Args...>);

		for (const std::size_t index : std::views::iota(std::size_t{ 0 }, defered_method.current_storage_index))
		{
			DecayTuple<Args...>* tuple = (DecayTuple<Args...>*)(defered_method.storage + STORAGE_SIZE * index);
			PassThrough(defered_method.instance, (void(*)(void*, Args...))defered_method.hooked_function, *tuple, std::index_sequence_for<Args...>());
			tuple->~tuple();
		}
	}

	template<auto func, typename Type, typename ...Args>
	void* InternalHook(void(Type::*)(Args...))
	{
		return (void*)(&DeferMethodCalls::DeferedMethodCaller<func, Type, Args...>);
	}

	template<auto func, typename ...Args>
	void* InternalHook(void(*)(Args...))
	{
		return (void*)(&DeferMethodCalls::DeferedMethodCallerStatic<func, Args...>);
	}

	template<auto func, typename Type, typename ...Args>
	void* InternalHookDirect(void(Type::*)(Args...))
	{
		return (void*)(&DeferMethodCalls::DirectMethodCaller<func, Type, Args...>);
	}

	template<auto func, typename ...Args>
	void* InternalHookDirect(void(*)(Args...))
	{
		return (void*)(&DeferMethodCalls::DirectMethodCallerStatic<func, Args...>);
	}

	template<typename Type, typename ...Args>
	void* HookClearBuffer(void(Type::*)(Args...))
	{
		return (void*)(&DeferMethodCalls::ClearDeferedMethodBuffer<Args...>);
	}

	template<typename ...Args>
	void* HookClearBuffer(void(*)(Args...))
	{
		return (void*)(&DeferMethodCalls::ClearDeferedMethodBuffer<Args...>);
	}

	template<typename Type, typename ...Args>
	char* AllocateStorage(void(Type::*)(Args...))
	{
		return (char*)malloc(STOREAGE_MAX_SIZE * sizeof(DecayTuple<Args...>));
	}

	template<typename ...Args>
	char* AllocateStorage(void(*)(Args...))
	{
		return (char*)malloc(STOREAGE_MAX_SIZE * sizeof(DecayTuple<Args...>));
	}

	std::vector<DeferedMethodData> m_defered_methods;
};