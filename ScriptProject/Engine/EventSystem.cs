using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.Remoting.Messaging;
using System.Text;
using System.Threading.Tasks;

namespace ScriptProject.Engine
{
    internal class EventSystem
    {


        //public delegate void Callback();
        public delegate void Callback<T>(T g);
        //public delegate void Callback<T, Y>(T g, Y f);
        //public delegate void Callback<T, Y, U>(T g, Y f);
        //public delegate void Callback<T, Y, U, I>(T g, Y f);
        //public delegate void Callback<T, Y, U, I, O>(T g, Y f);

        public class EventThing
        {

        }

        public class OrcEvent : EventThing
        {
            Callback<int> callback;
        }

        public class OrcEventSender
        {

        }

        public class EventData
        {
            public event EventHandler event_handler;
            //public Callback callback = null;
            //public Callback<T> callback_1 = null;
            public Dictionary<UInt32, Dictionary<UInt32, ScriptingBehaviour>> scene_entity_to_scripting_behaviour = new Dictionary<uint, Dictionary<uint, ScriptingBehaviour>>();
            public HashSet<ScriptingBehaviour> event_recievers = new HashSet<ScriptingBehaviour>();
        }

        static private Dictionary<string, EventData> events = new Dictionary<string, EventData>();

        public static EventData HookEvent(string name, EventHandler hook_event)
        {
            EventData event_data = new EventData();
            event_data.event_handler += hook_event;
            events.Add(name, event_data);

            return event_data;
        }

        public static void m()
        {
            HookEvent("name", sdf).event_handler += sdf;
        }

        public static void sdf(object sender, EventArgs e)
        {
            
        }
    }
}
