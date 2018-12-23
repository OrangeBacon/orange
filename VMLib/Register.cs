using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace VMLib {
    public class Register : Component {
        public Int16 Value { get; private set; }

        private readonly Bus bus;
        private readonly VMCore core;

        public Register(VMCore core, string name, Bus bus) : base(core, nameof(Register) +" "+ name) {
            Commands.Add(new MicrocodeCommand($"Store Register {name}", Store, this) {
                Depends = {bus},
                Changes = {this}
            });
            Commands.Add(new MicrocodeCommand($"Load Register {name}", Load, this) {
                Depends = {this},
                Changes = {bus}
            });
            OnPropertyChanged("Value");
            this.bus = bus;
            this.core = core;
        }

        private void Store() {
            Value = bus.Read();
            OnPropertyChanged("Value");
        }

        private void Load() {
            bus.Write(Value);
        }
    }
}
