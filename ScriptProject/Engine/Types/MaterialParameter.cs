using System;
using System.Runtime.CompilerServices;

namespace ScriptProject.Engine.Types
{
    internal class MaterialParameter
    {
        private UInt32 material_index;
        private UInt32 material_parameter_index;

        public MaterialParameter(UInt32 material_index, UInt32 material_parameter_index) { this.material_index = material_index; this.material_parameter_index = material_parameter_index; }

        [MethodImpl(MethodImplOptions.InternalCall)]
        static private extern void SetFloat_External(UInt32 material_index, UInt32 material_parameter_index, float value);

        public void SetFloat(float value) { SetFloat_External(material_index, material_parameter_index, value); }
    }
}
