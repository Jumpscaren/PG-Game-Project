using System;
using System.Collections.Generic;
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

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        private static extern void AddUserPrefab(UInt32 prefab_instance_id, UInt32 z_index);

        public static void CreateUserPrefab(Action<GameObject> prefab_instance_method, UInt32 z_index)
        {
            //Need to be changed later maybe
            m_prefab_actions.Add(prefab_instance_method);

            AddUserPrefab(m_pid, z_index);
            ++m_pid;
        }

        private static void InstanceUserPrefab(GameObject gameObject, UInt32 prefab_instance_id)
        {
            m_prefab_actions[(int)prefab_instance_id](gameObject);
        }
    }
}
