#pragma once
class EventCore
{
private:

	struct HookedEventCallbackData
	{
		void* hooked_callback_method;
		void* original_method_ptr;
		uint8_t event_order;
	};

	struct EventListenerData
	{
		std::vector<HookedEventCallbackData> hooked_event_callback_data;
		uint8_t number_of_arguments;
#ifdef _DEBUG
		std::vector<std::string> argument_types;
#endif // _DEBUG
	};

private:
	std::unordered_map<uint64_t, EventListenerData> m_event_listeners;
	static EventCore* s_event_core;

private:
	template<typename ...Args>
	void CheckEventData(const EventListenerData& event_listener_data);

	template<void* hooked_method, typename ...Args>
	static void HookEvent(const Args& ...args);

public:
	EventCore();
	static EventCore* Get();

	template<typename ...Args>
	void SendEvent(const std::string& event_name, const Args& ...args);

	template<auto t_method, typename ...Args>
	void ListenToEvent(const std::string& event_name, const uint8_t event_order, void(*)(Args...));
};

template<typename ...Args>
inline void EventCore::CheckEventData(const EventListenerData& event_listener_data)
{
	//Different number of arguments
	assert(event_listener_data.number_of_arguments == sizeof...(Args));

#ifdef _DEBUG
	std::vector<std::string> argument_types;
	(argument_types.push_back(typeid(Args).name()), ...);

	for (uint64_t i = 0; i < argument_types.size(); ++i)
	{
		if (argument_types[i] != event_listener_data.argument_types[i])
		{
			std::cout << "Argument " << (i + 1) << "; Callback Argument Type = " << event_listener_data.argument_types[i] << ", Incoming Argument Type = " << argument_types[i] << "\n";
			assert(false);
		}
	}
#endif // _DEBUG
}

template<void* hooked_method, typename ...Args>
inline void EventCore::HookEvent(const Args& ...args)
{
	((void(*)(Args...))(hooked_method))(args...);
}

template<typename ...Args>
inline void EventCore::SendEvent(const std::string& event_name, const Args& ...args)
{
	uint64_t hased_event_name = std::hash<std::string>{}(event_name);
	auto it = m_event_listeners.find(hased_event_name);

	if (it == m_event_listeners.end())
	{
		//assert(false);
		std::cout << "NO LISTENERS FOR THIS EVENT: '" << event_name << "'\n";
	}
	else
	{
		const EventListenerData& event_listener_data = it->second;
#ifdef _DEBUG
		CheckEventData<Args...>(event_listener_data);
#endif // _DEBUG
		for (uint64_t i = 0; i < event_listener_data.hooked_event_callback_data.size(); ++i)
		{
			((void(*)(const Args&...))(event_listener_data.hooked_event_callback_data[i].hooked_callback_method))(args...);
		}
	}
}

template<auto t_method, typename ...Args>
inline void EventCore::ListenToEvent(const std::string& event_name, const uint8_t event_order, void(*)(Args ...))
{
	uint64_t hased_event_name = std::hash<std::string>{}(event_name);
	auto it = m_event_listeners.find(hased_event_name);
	
	if (it == m_event_listeners.end())
	{
		//New event listened to
		EventListenerData first_event_listener_data = {};
		first_event_listener_data.number_of_arguments = sizeof...(Args);

#ifdef _DEBUG
		(first_event_listener_data.argument_types.push_back(typeid(Args).name()), ...);
#endif // _DEBUG

		m_event_listeners.insert({hased_event_name, first_event_listener_data });

		it = m_event_listeners.find(hased_event_name);
	}
	else
	{
#ifdef _DEBUG
		const EventListenerData& event_listener_data = it->second;
		CheckEventData<Args...>(event_listener_data);
		for (uint64_t i = 0; i < event_listener_data.hooked_event_callback_data.size(); ++i)
		{
			assert(event_listener_data.hooked_event_callback_data[i].original_method_ptr != (void*)t_method);
		}
#endif // _DEBUG

	}

	HookedEventCallbackData new_callback = {};
	new_callback.hooked_callback_method = (void*)EventCore::HookEvent<(void*)t_method, Args...>;
	new_callback.original_method_ptr = (void*)t_method;
	new_callback.event_order = event_order;
	auto& callback_data = it->second.hooked_event_callback_data;
	auto const new_pos = std::lower_bound(callback_data.begin(), callback_data.end(), new_callback, [](const HookedEventCallbackData& first, const HookedEventCallbackData& second)
		{
			return first.event_order < second.event_order;
		});
	it->second.hooked_event_callback_data.insert(new_pos, new_callback);
}
