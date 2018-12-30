using System.Collections.ObjectModel;

namespace VMLib {

    // main component of the virtual machine, everything else depends on this
    // being initialised
    public class VMCore : ObservableObject {

        // maintain list of components so it can be read by interface
        private readonly ObservableCollection<Component> _components = new ObservableCollection<Component>();
        public ReadOnlyObservableCollection<Component> Components { get; }

        public Clock Clock { get; }
        public MicrocodeController Controller { get; }

        public static ILogger Log = new NullLogger();

        public VMCore() {
            Components = new ReadOnlyObservableCollection<Component>(_components);
            Clock = new Clock();
            Controller = new MicrocodeController(this);
            Controller.PropertyChanged += (a, b) => { OnPropertyChanged("Controller"); };
        }

        public void Add(Component c) {
            _components.Add(c);
            OnPropertyChanged("Components");
            Controller.Add(c);
            Log.Info("Added Component");
        }
    }
}
