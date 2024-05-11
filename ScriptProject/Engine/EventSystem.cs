using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Runtime.Remoting.Messaging;
using System.Text;
using System.Threading.Tasks;

namespace ScriptProject.Engine
{
    internal class EventSystem
    {
        public class BaseEventData
        {

        }

        public delegate void BaseEventHandler(BaseEventData event_data);
        static Dictionary<string, Dictionary<UInt32, BaseEventHandler>> events = new Dictionary<string, Dictionary<UInt32, BaseEventHandler>>();

        public static void ListenToEvent(string event_name, GameObject game_object, BaseEventHandler event_hook)
        {
            Dictionary<UInt32, BaseEventHandler> listen_hooks;
            if (!events.TryGetValue(event_name, out listen_hooks))
            {
                listen_hooks = new Dictionary<UInt32, BaseEventHandler>();
                events.Add(event_name, listen_hooks);
            }
            listen_hooks.Add(game_object.GetEntityID(), event_hook);
        }

        public static void StopListeningToEvent(string event_name, GameObject game_object, BaseEventHandler event_hook)
        {
            Dictionary<UInt32, BaseEventHandler> listen_hooks;
            if (events.TryGetValue(event_name, out listen_hooks))
            {
                listen_hooks.Remove(game_object.GetEntityID());
            }
        }

        public static void SendEvent(string event_name, BaseEventData event_data)
        {
            Dictionary<UInt32, BaseEventHandler> listen_hooks;
            if (events.TryGetValue(event_name, out listen_hooks))
            {
                foreach (var hooks in listen_hooks)
                {
                    hooks.Value(event_data);
                }
            }
        }
    }
}
