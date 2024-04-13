using System;
using System.Collections.Generic;
using System.Data;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Security.Cryptography;
using System.Text;
using System.Threading.Tasks;

namespace ScriptProject.Engine
{
    internal class PrefabSystem
    {
        static UInt32 m_pid = 0;

        static List<Action<GameObject>> m_prefab_actions = new List<Action<GameObject>>();
        static Dictionary<string, UInt32> m_name_to_index = new Dictionary<string, uint>(); 

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        private static extern void AddUserPrefab(string prefab_name, UInt32 z_index);

        public static void CreateUserPrefab(string prefab_name, Action<GameObject> prefab_instance_method, UInt32 z_index)
        {
            //Need to be changed later maybe
            m_prefab_actions.Add(prefab_instance_method);

            m_name_to_index.Add(prefab_name, m_pid);
            AddUserPrefab(prefab_name, z_index);
            ++m_pid;
        }

        public static void InstanceUserPrefab(GameObject gameObject, string prefab_name)
        {
            if (m_name_to_index.ContainsKey(prefab_name))
            {
                UInt32 prefab_instance_id = m_name_to_index[prefab_name];
                m_prefab_actions[(int)prefab_instance_id](gameObject);
            }
        }
    }
}
