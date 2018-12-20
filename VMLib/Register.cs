using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace VMLib {
    public class Register : Component {
        public int Value { get; private set; }

        private readonly Bus bus;
        private readonly VMCore core;

        public Register(VMCore core, string name, Bus bus) : base(core) {
            Commands.Add(new MicrocodeCommand($"Store Register {name}", Store));
            Commands.Add(new MicrocodeCommand($"Load Register {name}", Load));
            OnPropertyChanged("Value");
            this.bus = bus;
            this.core = core;
        }

        private void Store() {
            Value = bus.Read();
        }

        private void Load() {
            bus.Write(Value);
        }
    }
}
