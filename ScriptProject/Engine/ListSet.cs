using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ScriptProject.Engine
{
    internal class ListSetGameObject
    {
        List<GameObject> data = new List<GameObject>();

        public void Add(GameObject new_data)
        {
            data.Add(new_data);
        }

        public List<GameObject> GetData()
        {
            return data;
        }
    }
}
