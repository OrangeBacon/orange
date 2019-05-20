using System.Collections.ObjectModel;

namespace VMLib {

    // base class for all components in the virtual machine
    public abstract class Component : ObservableObject {
        private static int ComponentIDCounter = 0;

        // unique id
        public int ComponentID { get; }

        // used to display the type of the component when displayed in the interface
        public string TypeName { get; private set; }

        public string Name { get; private set; }

        public ObservableCollection<MicrocodeCommand> Commands { get; } = new ObservableCollection<MicrocodeCommand>();

        // register the component with the virtual machine
        public Component(VMCore core, string name) {
            Name = name;
            TypeName = GetType().Name;
            core.Add(this);
            ComponentID = ComponentIDCounter++;
        }
    }
}
