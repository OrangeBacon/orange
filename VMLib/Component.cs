using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Runtime.CompilerServices;
using System.Text;

namespace VMLib {
    public abstract class Component : ObservableObject {
        private static int ComponentIDCounter = 0;
        public int ComponentID { get; }

        public string TypeName { get; private set; }
        public string Name { get; private set; }

        public ObservableCollection<MicrocodeCommand> Commands { get; } = new ObservableCollection<MicrocodeCommand>();

        public Component(VMCore core, string name) {
            Name = name;
            TypeName = GetType().Name;
            core.Add(this);
            ComponentID = ComponentIDCounter++;
        }
    }
}
