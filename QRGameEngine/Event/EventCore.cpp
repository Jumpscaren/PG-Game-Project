#include "pch.h"
#include "EventCore.h"

EventCore* EventCore::s_event_core = nullptr;

EventCore::EventCore()
{
	s_event_core = this;
}

EventCore* EventCore::Get()
{
	return s_event_core;
}
