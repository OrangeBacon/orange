using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace VMLib {
    public class MicrocodeController : ObservableObject {
        private ILogger _log;

        public ObservableCollection<MicrocodeCommand> Commands { get; } = new ObservableCollection<MicrocodeCommand>();

        public MicrocodeController(VMCore core, ILogger logger) {
            _log = logger;
            core.PropertyChanged += (object sender, PropertyChangedEventArgs e) => {
                if (e.PropertyName == "Log") {
                    _log = core.Log;
                }
            };

            Commands.CollectionChanged += (a, b) => System.Diagnostics.Trace.WriteLine("CHANGED!");
        }

        public void Add(Component c) {
            foreach(MicrocodeCommand command in c.Commands) {
                Commands.Add(command);
            }
            c.Commands.CollectionChanged += (a, b) => {
                foreach (var item in b.NewItems) {
                    Commands.Add(item as MicrocodeCommand);
                }
            };
            OnPropertyChanged("Commands");
            _log.Info("Added Command");
        }
    }
}
