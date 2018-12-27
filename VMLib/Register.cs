using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using static VMLib.Util;

namespace VMLib {
    public class Register : Component {
        public short Value { get; private set; }
        private readonly VMCore core;

        public Register(VMCore core, string name, params LogicState<Bus>[] states) : base(core, nameof(Register) + " " + name) {
            foreach(var state in states) {
                var bus = state.Value;
                if((state.State & LogicState.In) == LogicState.In)
                Commands.Add(new MicrocodeCommand($"{bus.Name} -> Register {name}", Bind(Store, bus), this) {
                    Depends = { bus },
                    Changes = { this }
                });

                if((state.State & LogicState.Out) == LogicState.Out)
                Commands.Add(new MicrocodeCommand($"Register {name} -> {bus.Name}", Bind(Load, bus), this) {
                    Depends = { this },
                    Changes = { bus }
                });
            }
            OnPropertyChanged("Value");
            this.core = core;
        }

        public void Store(Bus bus) {
            Value = bus.Read();
            OnPropertyChanged("Value");
        }
        public void Store(short val) {
            Value = val;
            OnPropertyChanged("Value");
        }

        public void Load(Bus bus) {
            bus.Write(Value);
        }
    }
}
