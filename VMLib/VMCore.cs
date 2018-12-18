using System.Collections.Generic;
using System.Collections.ObjectModel;

namespace VMLib {
    public class VMCore {
        readonly List<Component> Comp = new List<Component>();
        public ReadOnlyCollection<Component> Components { get => Comp.AsReadOnly(); }
        public ILogger Log = new NullLogger();
        public readonly MicrocodeController controller = new MicrocodeController();
        
        public void Add(Component c) {
            Comp.Add(c);
            controller.AddRange(c.Commands);
            Log.Info("Added Component");
        }
    }
}
