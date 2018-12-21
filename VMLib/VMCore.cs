using System.Collections.Generic;
using System.Collections.ObjectModel;

namespace VMLib {
    public class VMCore : ObservableObject {
        private readonly ObservableCollection<Component> _components = new ObservableCollection<Component>();
        public ReadOnlyObservableCollection<Component> Components { get; }

        public MicrocodeController Controller { get; }

        private ILogger _log = new NullLogger();
        public ILogger Log { get=>_log; set { _log = value; OnPropertyChanged(); } }

        public VMCore() {
            Components = new ReadOnlyObservableCollection<Component>(_components);
            Controller = new MicrocodeController(this, Log);
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
