using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace VMLib {
    public class Bus : Component {
        public short Value { get; private set; }

        public Bus(VMCore core, string name) : base(core, name + " " + nameof(Bus)) { }

        public short Read() {
            return Value;
        }

        public void Write(short val) {
            Value = val;
            OnPropertyChanged("Value");
        }
    }
}
