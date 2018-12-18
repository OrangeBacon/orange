using System.Collections.Generic;
using System.Collections.ObjectModel;

namespace VMLib {
    public class VMCore {
        readonly List<Component> Comp = new List<Component>();
        public ReadOnlyCollection<Component> Components { get => Comp.AsReadOnly(); }
        private readonly ILogger Log = new NullLogger();
        
        public void Add(Component c) {
            Comp.Add(c);
            Log.Info("Added Component");
        }
    }
}
