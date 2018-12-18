using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace VMLib {
    public class Register : Component {
        public int Value { get; private set; }

        public static string TypeName { get; } = nameof(Register);

        public Register(VMCore core, string name) : base(core) {
            Commands.Add(new MicrocodeCommand($"Store Register {name}"));
            Commands.Add(new MicrocodeCommand($"Load Register {name}"));
            OnPropertyChanged("Value");
        }
    }
}
