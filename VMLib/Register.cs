using static VMLib.Util;

namespace VMLib {
    // wrapper around storing a value and outputting it to buses
    public class Register : Component {
        public short Value { get; private set; }

        public Register(VMCore core, string name, params LogicState<Bus>[] states) : base(core, nameof(Register) + " " + name) {
            foreach(var state in states) {
                var bus = state.Value;

                // if input allowed
                if((state.State & LogicState.In) == LogicState.In)
                Commands.Add(new MicrocodeCommand($"{bus.Name} -> Register {name}", Bind(Store, bus)) {
                    Depends = { bus },
                    Changes = { this }
                });

                // if output allowed
                if((state.State & LogicState.Out) == LogicState.Out)
                Commands.Add(new MicrocodeCommand($"Register {name} -> {bus.Name}", Bind(Load, bus)) {
                    Depends = { this },
                    Changes = { bus }
                });
            }
            OnPropertyChanged("Value");
        }

        public void Store(Bus bus) {
            Value = bus.Read();
            OnPropertyChanged("Value");
        }

        public void Load(Bus bus) {
            bus.Write(Value);
        }
    }
}
