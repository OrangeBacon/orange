using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace VMLib {
    public class Register : Component {
        public int Value { get => Value; }
        private int value;
        

        public Register(VMCore core) : base(core) {
            Commands.Add(new MicrocodeCommand());
            Commands.Add(new MicrocodeCommand());
        }

    }
}
