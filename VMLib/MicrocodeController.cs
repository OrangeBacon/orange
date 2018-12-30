using System.Collections.ObjectModel;
using static VMLib.VMCore;

namespace VMLib {

    // container around microcode commands
    public class MicrocodeController : ObservableObject {
        public ObservableCollection<MicrocodeCommand> Commands { get; } = new ObservableCollection<MicrocodeCommand>();

        // commands that should be run on the clock tick
        public ObservableCollection<MicrocodeCommand> ActiveCommands { get; } = new ObservableCollection<MicrocodeCommand>();
        public Graph<MicrocodeCommand> ActiveGraph = new Graph<MicrocodeCommand>();

        public MicrocodeController(VMCore core) {
            // make sure that the command graph is always correct
            ActiveCommands.CollectionChanged += (a, b) => {
                UpdateGraph();
            };

            // run the commands in the graph
            core.Clock.At(1).Add(() => {
                foreach(var c in ActiveGraph.TopologicalSort()) {
                    c.Run();
                }
            });
        }

        // add a new component's commands to the controller
        // TODO: do nothing if component already added
        public void Add(Component c) {
            foreach(var command in c.Commands) {
                Commands.Add(command);
            }

            // when this function is called, the component's constructor will not have been called,
            // as this is run by abstract component's constructor.  Therefore, it is likley
            // that commands will be added after the component is added to this class
            c.Commands.CollectionChanged += (a, b) => {
                foreach(var item in b.NewItems) {
                    Commands.Add(item as MicrocodeCommand);
                }
            };
        }

        // recreate the dependancy graph
        private void UpdateGraph() {
            var graph = new Graph<MicrocodeCommand>();

            // loop through all commands that should be in the graph
            foreach(var command in ActiveCommands) {
                // and add them
                graph.AddNode(command);

                // then add all edges that lead out from that command towards another command
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
    }
}
