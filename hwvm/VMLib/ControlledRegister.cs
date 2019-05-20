using static VMLib.Util;

namespace VMLib {
    // wrapper around storing a value and outputting it to buses
    public class ControlledRegister : Component {
        public ushort Value { get; private set; }

        public ControlledRegister(VMCore core, string name) : base(core, nameof(Register) + " " + name) {
            OnPropertyChanged("Value");
        }

        public void In(Bus bus) {
            Value = bus.Read();
            OnPropertyChanged("Value");
        }

        public void Out(Bus bus) {
            bus.Write(Value);
        }
    }
}
