using System;
using System.Collections.Generic;

namespace VMLib {

    // wrapper around a action including the components that the
    // command changes or depends on
    public class MicrocodeCommand : ObservableObject {
        public string Name { get; }

        public List<Component> Depends { get; set; } = new List<Component>();
        public List<Component> Changes { get; set; } = new List<Component>();

        private readonly Action action;

        public MicrocodeCommand(string name, Action action) {
            Name = name;
            this.action = action;
        }

        public void Run() {
            action?.Invoke();
        }
    }
}
