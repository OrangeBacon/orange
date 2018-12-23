using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace VMLib {
    public class MicrocodeCommand : ObservableObject {
        public string Name { get; }

        public List<Component> Depends { get; set; } = new List<Component>();
        public List<Component> Changes { get; set; } = new List<Component>();

        public Component Owner;

        private readonly Action action;

        public MicrocodeCommand(string name, Action action, Component owner) {
            Name = name;
            this.action = action;
            Owner = owner;
        }

        public void Run() {
            action();
        }
    }
}
