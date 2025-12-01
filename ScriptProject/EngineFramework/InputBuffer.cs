using ScriptProject.Engine;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using static ScriptProject.Engine.Input;

namespace ScriptProject.EngineFramework
{
    internal class InputBuffer
    {
        enum BufferedInputType
        {
            None,
            Key,
            MouseButton,
            MouseWheelSpin
        }

        struct BufferedInput
        {
            public BufferedInputType buffered_input_type;

            public Input.Key key;
            public Input.MouseButton mouse_button;
            public Input.MouseWheelSpin wheel_spin;

            public static bool operator ==(BufferedInput left, BufferedInput right)
            {
                if (ReferenceEquals(left, right))
                {
                    return true;
                }

                if (left.buffered_input_type != right.buffered_input_type)
                {
                    return false;
                }

                if (left.key != Key.END_ENUM && left.key == right.key)
                {
                    return true;
                }

                if (left.mouse_button != MouseButton.END_ENUM && left.mouse_button == right.mouse_button)
                {
                    return true;
                }

                if (left.wheel_spin != MouseWheelSpin.END_ENUM && left.wheel_spin == right.wheel_spin)
                {
                    return true;
                }

                return false;
            }
            public static bool operator !=(BufferedInput left, BufferedInput right)
            {
                return !(left == right);
            }
            public override bool Equals(object obj)
            {
                if (obj.GetType() != typeof(BufferedInput))
                {
                    return false;
                }

                return this == (BufferedInput)obj;
            }
            public override int GetHashCode()
            {
                if (buffered_input_type == BufferedInputType.Key)
                {
                    return 0x00010000 + ((int)key);
                }
                if (buffered_input_type == BufferedInputType.MouseWheelSpin)
                {
                    return 0x00100000 + ((int)wheel_spin);
                }
                return 0x01000000 + ((int)mouse_button);
            }
        }

        BufferedInput empty_buffered_input = new BufferedInput{buffered_input_type = BufferedInputType.None, key = Input.Key.END_ENUM, mouse_button = Input.MouseButton.END_ENUM, wheel_spin = Input.MouseWheelSpin.END_ENUM};

        Dictionary<BufferedInput, Timer> buffered_inputs = new Dictionary<BufferedInput, Timer>();

        void AddBufferedInput(BufferedInput buffered_input, float buffering_time)
        {
            if (buffered_inputs.ContainsKey(buffered_input))
            {
                buffered_inputs[buffered_input].SetTimeLimit(buffering_time);
                buffered_inputs[buffered_input].Start();
                return;
            }

            Timer buffer_timer = new Timer();
            buffer_timer.SetTimeLimit(buffering_time);
            buffer_timer.Start();
            buffered_inputs.Add(buffered_input, buffer_timer);
        }

        public void AddBufferedInput(Input.Key key, float buffering_time)
        {
            BufferedInput buffered_input = empty_buffered_input;
            buffered_input.buffered_input_type = BufferedInputType.Key;
            buffered_input.key = key;

            AddBufferedInput(buffered_input, buffering_time);
        }

        public void AddBufferedInput(Input.MouseButton mouse_button, float buffering_time)
        {
            BufferedInput buffered_input = empty_buffered_input;
            buffered_input.buffered_input_type = BufferedInputType.MouseButton;
            buffered_input.mouse_button = mouse_button;

            AddBufferedInput(buffered_input, buffering_time);   
        }

        bool ConsumeBufferedInput(BufferedInput buffered_input)
        {
            if (!buffered_inputs.ContainsKey(buffered_input))
            {
                return false;
            }

            bool is_expired = buffered_inputs[buffered_input].IsExpired();
            buffered_inputs.Remove(buffered_input);
            return !is_expired;
        }

        public bool ConsumeBufferedInput(Input.Key key)
        {
            BufferedInput buffered_input = empty_buffered_input;
            buffered_input.buffered_input_type = BufferedInputType.Key;
            buffered_input.key = key;

            return ConsumeBufferedInput(buffered_input);
        }

        public bool ConsumeBufferedInput(Input.MouseButton mouse_button)
        {
            BufferedInput buffered_input = empty_buffered_input;
            buffered_input.buffered_input_type = BufferedInputType.MouseButton;
            buffered_input.mouse_button = mouse_button;

            return ConsumeBufferedInput(buffered_input);
        }

        bool CheckBufferedInput(BufferedInput buffered_input)
        {
            if (!buffered_inputs.ContainsKey(buffered_input))
            {
                return false;
            }

            bool is_expired = buffered_inputs[buffered_input].IsExpired();
            return !is_expired;
        }

        public bool CheckBufferedInput(Input.MouseButton mouse_button)
        {
            BufferedInput buffered_input = empty_buffered_input;
            buffered_input.buffered_input_type = BufferedInputType.MouseButton;
            buffered_input.mouse_button = mouse_button;

            return CheckBufferedInput(buffered_input);
        }
    }
}
