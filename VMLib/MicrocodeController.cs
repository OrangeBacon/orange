using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using static VMLib.VMCore;

namespace VMLib {
    public class MicrocodeController : ObservableObject {
        public ObservableCollection<MicrocodeCommand> Commands { get; } = new ObservableCollection<MicrocodeCommand>();

        public ObservableCollection<MicrocodeCommand> ActiveCommands { get; } = new ObservableCollection<MicrocodeCommand>();
        public Graph<MicrocodeCommand> ActiveGraph = new Graph<MicrocodeCommand>();

        private readonly VMCore Core;
        public MicrocodeController(VMCore core) {
            Core = core;

            ActiveCommands.CollectionChanged += (a, b) => {
                UpdateGraph();
            };
        }

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
            Log.Info("Added Command");
        }

        private void UpdateGraph() {
            var graph = new Graph<MicrocodeCommand>();
            foreach(var command in ActiveCommands) {
                graph.Nodes.Add(command);
                foreach(var changed in command.Changes) {
                    foreach(var comm in ActiveCommands) {
                        foreach(var depended in comm.Depends) {
                            if(changed == depended)
                                graph.AddEdge(command, comm);
                        }
                    }
                }
            }
            ActiveGraph = graph;
        }

        public void RunActive() {
            foreach(var c in ActiveGraph.TopologicalSort()) {
                c.Run();
            }
            foreach(var c in Core.Components) {
                c.OnClockTick();
            }
        }
    }
}
