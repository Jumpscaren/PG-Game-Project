#pragma once
#include "CSMonoClass.h"
#include "CSMonoMethod.h"
#include "CSMonoObject.h"
#include "CSMonoString.h"
#include <mutex>

struct _MonoDomain;
struct _MonoAssembly;
struct _MonoImage;
struct _MonoThread;

//To force garbage collection to remove old objects
//mono_gc_collect(mono_gc_max_generation());

namespace MonoCore
{ 
	template <typename T>
	concept IsOfBaseTypes = std::is_same_v <T, int> || std::is_same_v <T, uint8_t> || std::is_same_v <T, uint16_t> || std::is_same_v <T, uint32_t> || std::is_same_v <T, uint64_t> || std::is_same_v <T, float> || std::is_same_v <T, double> || std::is_same_v <T, bool>;
}

class CSMonoCore
{
	friend CSMonoObject;
	friend CSMonoClass;

public:
	static constexpr MonoMethodHandle NULL_METHOD = { (uint64_t)(-1) };
	static constexpr MonoClassHandle NULL_CLASS = { (uint64_t)(-1) };

private:
	typedef enum {
		MONO_TYPE_END = 0x00,       /* End of List */
		MONO_TYPE_VOID = 0x01,
		MONO_TYPE_BOOLEAN = 0x02,
		MONO_TYPE_CHAR = 0x03,
		MONO_TYPE_I1 = 0x04,
		MONO_TYPE_U1 = 0x05,
		MONO_TYPE_I2 = 0x06,
		MONO_TYPE_U2 = 0x07,
		MONO_TYPE_I4 = 0x08,
		MONO_TYPE_U4 = 0x09,
		MONO_TYPE_I8 = 0x0a,
		MONO_TYPE_U8 = 0x0b,
		MONO_TYPE_R4 = 0x0c,
		MONO_TYPE_R8 = 0x0d,
		MONO_TYPE_STRING = 0x0e,
		MONO_TYPE_PTR = 0x0f,       /* arg: <type> token */
		MONO_TYPE_BYREF = 0x10,       /* arg: <type> token */
		MONO_TYPE_VALUETYPE = 0x11,       /* arg: <type> token */
		MONO_TYPE_CLASS = 0x12,       /* arg: <type> token */
		MONO_TYPE_VAR = 0x13,	   /* number */
		MONO_TYPE_ARRAY = 0x14,       /* type, rank, boundsCount, bound1, loCount, lo1 */
		MONO_TYPE_GENERICINST = 0x15,	   /* <type> <type-arg-count> <type-1> \x{2026} <type-n> */
		MONO_TYPE_TYPEDBYREF = 0x16,
		MONO_TYPE_I = 0x18,
		MONO_TYPE_U = 0x19,
		MONO_TYPE_FNPTR = 0x1b,	      /* arg: full method signature */
		MONO_TYPE_OBJECT = 0x1c,
		MONO_TYPE_SZARRAY = 0x1d,       /* 0-based one-dim-array */
		MONO_TYPE_MVAR = 0x1e,       /* number */
		MONO_TYPE_CMOD_REQD = 0x1f,       /* arg: typedef or typeref token */
		MONO_TYPE_CMOD_OPT = 0x20,       /* optional arg: typedef or typref token */
		MONO_TYPE_INTERNAL = 0x21,       /* CLR internal type */

		MONO_TYPE_MODIFIER = 0x40,       /* Or with the following types */
		MONO_TYPE_SENTINEL = 0x41,       /* Sentinel for varargs method signature */
		MONO_TYPE_PINNED = 0x45,       /* Local var that points to pinned object */

		MONO_TYPE_ENUM = 0x55        /* an enumeration */
	} CSMonoType;

	struct MonoClassHandlerHasher
	{
		std::size_t operator()(const MonoClassHandle& mono_class_handle) const
		{
			return mono_class_handle.handle;
		}
	};

	struct MonoThreadHandlerHasher
	{
		std::size_t operator()(const MonoThreadHandle& mono_thread_handle) const
		{
			return mono_thread_handle.handle;
		}
	};

private:
	_MonoDomain* m_domain;
	_MonoAssembly* m_assembly;
	_MonoImage* m_image;

	qr::unordered_map<_MonoClass*, MonoClassHandle> m_mono_class_ptr_to_mono_class_handle;
	//qr::unordered_map<MonoClassHandle, std::mutex*, MonoClassHandlerHasher> m_mono_class_to_mutex;
	struct T
	{
		std::mutex mutex;
		std::atomic<std::thread::id> thread_id;
	} thread_mutex;
	std::vector<CSMonoClass> m_mono_classes;
	std::vector<CSMonoMethod> m_mono_methods;
	std::vector<uint32_t> m_mono_field_tokens;

	qr::unordered_map<std::string, MonoClassHandle> m_mono_class_name_to_mono_class_handle;
	qr::unordered_map<std::string, MonoMethodHandle> m_mono_method_name_to_mono_method_handle;

	static CSMonoCore* s_mono_core;

	std::vector<_MonoThread*> mono_threads;
	std::vector<MonoThreadHandle> free_mono_thread_handles;

private:
	//Type changing templates
	//Check out tuple from STD to check where this code came from

	/* 
	* Reason behind this is to let the user have simple types like string and not MonoString or CSMonoString.
	* With no type changing then the user would need to have pointers to a CSMonoString and CSMonoObject which could lead to memory leaks, in their functions which they hook.
	* The user would be responsible for removing the objects!
	* This is also much cleaner ;).
	*/

	struct ConstMonoString {};
	struct ConstMonoObject {};

	//Base struct
	template <char h, class T = void>
	struct TypeChange {};

	//If the type is a string then we want a _MonoString*
	template<>
	struct TypeChange<0, std::string>
	{
		using type = _MonoString*;
	};

	//If the type is a string then we want a _MonoString*
	template<>
	struct TypeChange<0, const std::string&>
	{
		using type = ConstMonoString*;
	};

	//If the type is a CSMonoObject then we want a _MonoObject*
	template<>
	struct TypeChange<0, CSMonoObject>
	{
		using type = _MonoObject*;
	};

	//If the type is a CSMonoObject then we want a _MonoObject*
	template<>
	struct TypeChange<0, const CSMonoObject&>
	{
		using type = ConstMonoObject*;
	};

	//If the type is a _MonoString* then we want a string
	template<>
	struct TypeChange<0, _MonoString*>
	{
		using type = std::string;
	};

	//If the type is a _MonoString* then we want a string
	template<>
	struct TypeChange<0, ConstMonoString*>
	{
		using type = const std::string&;
	};

	//If the type is a _MonoObject* then we want a CSMonoObject
	template<>
	struct TypeChange<0, _MonoObject*>
	{
		using type = CSMonoObject;
	};

	//If the type is a _MonoObject* then we want a CSMonoObject
	template<>
	struct TypeChange<0, ConstMonoObject*>
	{
		using type = const CSMonoObject&;
	};


	//This is for types which we do not need to change
	template <class T>
	struct TypeChange<0, T>
	{
		using type = T;
	};

	template<class T>
	using ChangeType = typename TypeChange<0, T>::type;

private:
	CSMonoClass* GetMonoClass(const MonoClassHandle class_handle);
	CSMonoMethod* GetMonoMethod(const MonoMethodHandle method_handle);
	_MonoDomain* GetDomain() const;
	_MonoImage* GetImage() const;

	void HandleException(_MonoObject* exception);

	template<typename T>
	requires MonoCore::IsOfBaseTypes<T>
	void* ToMethodParameter(const T& data)
	{
		return (void*)&data;
	}
	void* ToMethodParameter(const char* string);
	void* ToMethodParameter(const std::string& string);
	void* ToMethodParameter(const CSMonoObject& mono_object);

	void* UnboxMonoObject(_MonoObject* mono_object);
	template<typename T>
	requires MonoCore::IsOfBaseTypes<T>
	T MonoObjectToValue(T* mono_object)
	{
		return *(T*)(UnboxMonoObject((_MonoObject*)mono_object));
	}
	std::string MonoObjectToValue(std::string* mono_object);
	CSMonoObject MonoObjectToValue(CSMonoObject* mono_object);

	template<typename T>
	requires MonoCore::IsOfBaseTypes<T>
	static T MonoMethodParameter(T mono_parameter)
	{
		return mono_parameter;
	}
	static int MonoMethodParameter(int mono_parameter) { return mono_parameter; }
	static std::string MonoMethodParameter(_MonoString* mono_parameter);
	static std::string MonoMethodParameter(ConstMonoString* mono_parameter);
	static CSMonoObject MonoMethodParameter(_MonoObject* mono_parameter);
	static CSMonoObject MonoMethodParameter(ConstMonoObject* mono_parameter);

	template<typename T>
	requires MonoCore::IsOfBaseTypes<T>
	static T MonoMethodReturn(T mono_return)
	{
		return mono_return;
	}
	static _MonoString* MonoMethodReturn(const std::string& mono_return);
	static _MonoObject* MonoMethodReturn(const CSMonoObject& mono_return);

	_MonoObject* CallMethodInternal(const MonoMethodHandle& method_handle, _MonoObject* mono_object, void** parameters, uint32_t parameter_count);

	MonoClassHandle RegisterMonoClass(_MonoClass* mono_class);

	bool IsValueTypeInternal(const CSMonoType cs_mono_type, const CSMonoObject& mono_object, const std::string& field_name);

	void* GetValueInternal(const CSMonoObject& mono_object, const std::string& field_name);
	void SetValueInternal(const CSMonoObject& mono_object, const std::string& field_name, void* value);
	void* GetValueInternal(const CSMonoObject& mono_object, const MonoFieldHandle& mono_field_handle);
	void SetValueInternal(const CSMonoObject& mono_object, const MonoFieldHandle& mono_field_handle, void* value);

	static std::string MonoStringToString(_MonoString* mono_string);
	//bool CheckIfParameterCountIsCorrect(uint32_t parameter_count, CS);

	//MonoMethodHandle FindMonoMethod();

	std::mutex* GetClassMutex(const MonoClassHandle mono_class_handle);

public:
	CSMonoCore();
	~CSMonoCore();

public:
	MonoClassHandle RegisterMonoClass(const std::string& class_namespace, const std::string& class_name);
	MonoMethodHandle RegisterMonoMethod(const MonoClassHandle& class_handle, const std::string& method_name);
	MonoFieldHandle RegisterField(const MonoClassHandle& mono_class_handle, const std::string& field_name);

	bool CheckIfMonoMethodExists(const MonoClassHandle& class_handle, const std::string& method_name);
	bool CheckIfMonoMethodExists(const MonoMethodHandle& method_class);

	MonoMethodHandle TryRegisterMonoMethod(const MonoClassHandle& class_handle, const std::string& method_name);
	MonoMethodHandle TryRegisterMonoMethod(const CSMonoObject& mono_object, const std::string& method_name);

	MonoClassHandle TryGetParentClass(const CSMonoObject& mono_object);

	template<typename T>
	bool IsValueType(const CSMonoObject& mono_object, const std::string& field_name);

	template<typename Type>
	void GetValue(Type& return_value, const CSMonoObject& mono_object, const std::string& field_name);
	template<typename Type>
	void SetValue(const Type& value_to_set, const CSMonoObject& mono_object, const std::string& field_name);
	template<typename Type>
	void GetValue(Type& return_value, const CSMonoObject& mono_object, const MonoFieldHandle& mono_field_handle);
	template<typename Type>
	void SetValue(const Type& value_to_set, const CSMonoObject& mono_object, const MonoFieldHandle& mono_field_handle);

	std::vector<std::string> GetAllFieldNames(const CSMonoObject& mono_object);

	void CallStaticMethod(const MonoMethodHandle method_handle);
	void CallMethod(const MonoMethodHandle method_handle, const CSMonoObject& mono_object);

	template<typename...Args>
	void CallStaticMethod(const MonoMethodHandle& method_handle, Args&& ...args);
	template<typename...Args>
	void CallMethod(const MonoMethodHandle& method_handle, const CSMonoObject& mono_object, Args&& ...args);

	template<typename Type>
	void CallStaticMethod(Type& return_value, const MonoMethodHandle& method_handle);
	template<typename Type>
	void CallMethod(Type& return_value, const MonoMethodHandle& method_handle, const CSMonoObject& mono_object);

	template<typename Type, typename...Args>
	void CallStaticMethod(Type& return_value, const MonoMethodHandle& method_handle, Args&& ...args);
	template<typename Type, typename...Args>
	void CallMethod(Type& return_value, const MonoMethodHandle& method_handle, const CSMonoObject& mono_object, Args&& ...args);

	void HookMethod(const MonoClassHandle class_handle, const std::string& method_name, const void* method);
	MonoMethodHandle HookAndRegisterMonoMethod(const MonoClassHandle& class_handle, const std::string& method_name, const void* method);

	template<auto t_method, typename Type, typename...Args>
	MonoMethodHandle HookAndRegisterMonoMethodType(const MonoClassHandle& class_handle, const std::string& method_name, Type(*)(Args...));

	template<auto t_method, typename...Args>
	MonoMethodHandle HookAndRegisterMonoMethodType(const MonoClassHandle& class_handle, const std::string& method_name, void(*)(Args...));

	template<void* method, typename Type, typename...Args>
	static Type HookedMethod(Args... args);

	template<void* method, typename...Args>
	static void HookedMethodVoid(Args... args);

	static CSMonoCore* Get();

	void PrintAllMethodsFromClass(const MonoClassHandle& class_handle);
	void PrintMethod(const MonoMethodHandle& method_handle);

	void ForceGarbageCollection();

	MonoThreadHandle HookThread();
	void UnhookThread(const MonoThreadHandle thread_handle);
};

template<typename T>
inline bool CSMonoCore::IsValueType(const CSMonoObject& mono_object, const std::string& field_name)
{
	assert(false);
	return IsValueTypeInternal(CSMonoType::MONO_TYPE_END, mono_object, field_name);
}

template<>
inline bool CSMonoCore::IsValueType<uint32_t>(const CSMonoObject& mono_object, const std::string& field_name)
{
	return IsValueTypeInternal(CSMonoType::MONO_TYPE_U4, mono_object, field_name);
}

template<>
inline bool CSMonoCore::IsValueType<float>(const CSMonoObject& mono_object, const std::string& field_name)
{
	return IsValueTypeInternal(CSMonoType::MONO_TYPE_R4, mono_object, field_name);
}

template<>
inline bool CSMonoCore::IsValueType<double>(const CSMonoObject& mono_object, const std::string& field_name)
{
	return IsValueTypeInternal(CSMonoType::MONO_TYPE_R8, mono_object, field_name);
}

template<>
inline bool CSMonoCore::IsValueType<bool>(const CSMonoObject& mono_object, const std::string& field_name)
{
	return IsValueTypeInternal(CSMonoType::MONO_TYPE_BOOLEAN, mono_object, field_name);
}

template<>
inline bool CSMonoCore::IsValueType<std::string>(const CSMonoObject& mono_object, const std::string& field_name)
{
	return IsValueTypeInternal(CSMonoType::MONO_TYPE_STRING, mono_object, field_name);
}

template<>
inline bool CSMonoCore::IsValueType<CSMonoObject>(const CSMonoObject& mono_object, const std::string& field_name)
{
	return IsValueTypeInternal(CSMonoType::MONO_TYPE_CLASS, mono_object, field_name);
}

template<typename Type>
inline void CSMonoCore::GetValue(Type& return_value, const CSMonoObject& mono_object, const std::string& field_name)
{
	void* value = GetValueInternal(mono_object, field_name);
	return_value = MonoObjectToValue((Type*)value);
}

template<typename Type>
inline void CSMonoCore::SetValue(const Type& value_to_set, const CSMonoObject& mono_object, const std::string& field_name)
{
	void* value = ToMethodParameter(value_to_set);
	SetValueInternal(mono_object, field_name, value);
}

template<typename Type>
inline void CSMonoCore::GetValue(Type& return_value, const CSMonoObject& mono_object, const MonoFieldHandle& mono_field_handle)
{
	void* value = GetValueInternal(mono_object, mono_field_handle);
	return_value = MonoObjectToValue((Type*)value);
}

template<typename Type>
inline void CSMonoCore::SetValue(const Type& value_to_set, const CSMonoObject& mono_object, const MonoFieldHandle& mono_field_handle)
{
	void* value = ToMethodParameter(value_to_set);
	SetValueInternal(mono_object, mono_field_handle, value);
}

template<typename ...Args>
inline void CSMonoCore::CallStaticMethod(const MonoMethodHandle& method_handle, Args && ...args)
{
	constexpr int argument_size = sizeof...(Args);

	void* parameters[argument_size];

	int index = 0;

	((parameters[index++] = ToMethodParameter(args)), ...);

	CallMethodInternal(method_handle, nullptr, parameters, index);
}

template<typename ...Args>
inline void CSMonoCore::CallMethod(const MonoMethodHandle& method_handle, const CSMonoObject& mono_object, Args && ...args)
{
	constexpr int argument_size = sizeof...(Args);

	void* parameters[argument_size];
	
	int index = 0;

	((parameters[index++] = ToMethodParameter(args)), ...);

	CallMethodInternal(method_handle, mono_object.GetMonoObject(), parameters, index);
}

template<typename Type>
inline void CSMonoCore::CallStaticMethod(Type& return_value, const MonoMethodHandle& method_handle)
{
	void** parameters = nullptr;

	_MonoObject* method_return_value = CallMethodInternal(method_handle, nullptr, parameters, 0);

	return_value = MonoObjectToValue((Type*)method_return_value);
}

template<typename Type>
inline void CSMonoCore::CallMethod(Type& return_value, const MonoMethodHandle& method_handle, const CSMonoObject& mono_object)
{
	void** parameters = nullptr;

	_MonoObject* method_return_value = CallMethodInternal(method_handle, mono_object.GetMonoObject(), parameters, 0);

	return_value = MonoObjectToValue((Type*)method_return_value);
}

template<typename Type, typename ...Args>
inline void CSMonoCore::CallStaticMethod(Type& return_value, const MonoMethodHandle& method_handle, Args && ...args)
{
	const int argument_size = sizeof...(Args);

	void* parameters[argument_size];

	int index = 0;

	((parameters[index++] = ToMethodParameter(args)), ...);

	_MonoObject* method_return_value = CallMethodInternal(method_handle, nullptr, parameters, index);

	return_value = MonoObjectToValue((Type*)method_return_value);
}

template<typename Type, typename ...Args>
inline void CSMonoCore::CallMethod(Type& return_value, const MonoMethodHandle& method_handle, const CSMonoObject& mono_object, Args && ...args)
{
	const int argument_size = sizeof...(Args);

	void* parameters[argument_size];

	int index = 0;

	((parameters[index++] = ToMethodParameter(args)), ...);

	_MonoObject* method_return_value = CallMethodInternal(method_handle, mono_object.GetMonoObject(), parameters, index);

	return_value = MonoObjectToValue((Type*)method_return_value);
}

template<auto t_method>
static MonoClassHandle SetAndGetHookedMonoClassHandle(const MonoClassHandle class_handle)
{
	static MonoClassHandle hooked_class_handle = class_handle;
	assert(class_handle.handle == 0 || (class_handle.handle != 0 && hooked_class_handle == class_handle));
	return hooked_class_handle;
}

template<auto t_method, typename Type, typename ...Args>
inline MonoMethodHandle CSMonoCore::HookAndRegisterMonoMethodType(const MonoClassHandle& class_handle, const std::string& method_name, Type(*)(Args...))
{
	SetAndGetHookedMonoClassHandle<t_method>(class_handle);
	return HookAndRegisterMonoMethod(class_handle, method_name, &CSMonoCore::HookedMethod<(void*)t_method, ChangeType<Type>, ChangeType<Args>...>);
}

template<auto t_method, typename ...Args>
inline MonoMethodHandle CSMonoCore::HookAndRegisterMonoMethodType(const MonoClassHandle& class_handle, const std::string& method_name, void(*)(Args ...))
{
	SetAndGetHookedMonoClassHandle<t_method>(class_handle);
	return HookAndRegisterMonoMethod(class_handle, method_name, &CSMonoCore::HookedMethodVoid<(void*)t_method, ChangeType<Args>...>);
}

template<void* method, typename Type, typename ...Args>
inline Type CSMonoCore::HookedMethod(Args ...args)
{
	return MonoMethodReturn(((ChangeType<Type>(*)(ChangeType<Args>...))(method))(MonoMethodParameter(args)...));
}

template<void* method, typename...Args>
inline void CSMonoCore::HookedMethodVoid(Args... args)
{
	((void(*)(ChangeType<Args>...))(method))(MonoMethodParameter(args)...);
}
