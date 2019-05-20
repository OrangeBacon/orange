using System.Collections.ObjectModel;

namespace VMLib {

    // main component of the virtual machine, everything else depends on this
    // being initialised
    public class VMCore : ObservableObject {

        // maintain list of components so it can be read by interface
        private readonly ObservableCollection<Component> _components = new ObservableCollection<Component>();
        public ReadOnlyObservableCollection<Component> Components { get; }

        // ensure certain core components have consistant locations
        public Clock Clock { get; }
        public MicrocodeController Controller { get; }
        public Flags Flags { get; }
        public Memory64k Memory { get; internal set; }

        public static ILogger Log = new NullLogger();

        public VMCore() {
            Components = new ReadOnlyObservableCollection<Component>(_components);

            Clock = new Clock();
            Controller = new MicrocodeController(this);
            Flags = new Flags(this);

            // ensure change propogation so everything updates in front end
            Controller.PropertyChanged += (a, b) => { OnPropertyChanged("Controller"); };
            Flags.PropertyChanged += (a, b) => {
                OnPropertyChanged("Flags");
            };
        }

        // register new component with the core
        // intended for use only from the constructor
        // of a component
        internal void Add(Component c) {
            _components.Add(c);
            OnPropertyChanged("Components");
            Controller.Add(c);
            Log.Info("Added Component");
        }
    }
}
