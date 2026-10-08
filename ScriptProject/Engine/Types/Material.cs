using System;
using System.Runtime.CompilerServices;

namespace ScriptProject.Engine.Types
{
    internal class Material
    {
        private UInt32 material_index;

        public Material(UInt32 material_index) { this.material_index = material_index; }

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        static private extern UInt32 GetMaterialParameter_External(UInt32 material_index, string material_parameter_name);

        public MaterialParameter GetMaterialParameter(string material_parameter_name) { return new MaterialParameter(material_index, GetMaterialParameter_External(material_index, material_parameter_name)); }
    }
}
