using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace VMLib {
    public class MicrocodeController : ObservableObject {
        public ObservableCollection<MicrocodeCommand> Commands { get; } = new ObservableCollection<MicrocodeCommand>();


        public void Add(Component c) {
            foreach(MicrocodeCommand command in c.Commands) {
                Commands.Add(command);
            }
            c.Commands.CollectionChanged += (a, b) => {
                foreach(var item in b.NewItems) {
                    Commands.Add(item as MicrocodeCommand);
                }
            };
            OnPropertyChanged("Commands");
            VMCore.Log.Info("Added Command");
        }

        public void Run(List<MicrocodeCommand> commands) {
            var graph = new Graph<MicrocodeCommand>();
            foreach(var command in commands) {
                graph.Nodes.Add(command);
                foreach(var changed in command.Changes) {
                    foreach(var comm in commands) {
                        foreach(var depended in comm.Depends) {
                            if(changed == depended)
                                graph.AddEdge(command, comm);
                        }
                    }
                }
            }
            foreach(var c in graph.TopologicalSort()) {
                c.Run();
            }
        }
    }
}
