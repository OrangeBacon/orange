using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace VMLib {
    public class Bus : Component {
        private int Value;

        public Bus(VMCore core) : base(core) { }

        public int Read() {
            return Value;
        }

        public void Write(int val) {
            Value = val;
        }
    }
}
