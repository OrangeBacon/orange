using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace VMLib {
    public class MicrocodeCommand {
        public string Name { get; }

        public bool Active { get; set; }

        private readonly Action action;

        public MicrocodeCommand(string name, Action action) {
            Name = name;
            this.action = action;
        }

        public void Run() {
            action();
        }
    }
}
