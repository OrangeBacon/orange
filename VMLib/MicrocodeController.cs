using System;
using System.Collections.ObjectModel;
using static VMLib.VMCore;

namespace VMLib {

    // container around microcode commands
    public class MicrocodeController : ObservableObject {
        public ObservableCollection<MicrocodeCommand> Commands { get; } = new ObservableCollection<MicrocodeCommand>();

        // commands that should be run on the clock tick
        public ObservableCollection<MicrocodeCommand> ActiveCommands { get; } = new ObservableCollection<MicrocodeCommand>();
        public Graph<MicrocodeCommand> ActiveGraph = new Graph<MicrocodeCommand>();

        public ObservableCollection<MicrocodeInput> Inputs { get; } = new ObservableCollection<MicrocodeInput>();

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

        // add the lower {bits} bits of {inp} to the input
        public void Input(Component component, string property, int bits) {
            var info = component.GetType().GetProperty(property);
            var value = info.GetValue(component, null);
            if(info.PropertyType.Name != nameof(UInt16)) {
                Log.Warn($"Input property, {property} is not a valid type (ushort)");
                return;
            }

            var inp = new MicrocodeInput {
                Value = (ushort)value,
                Bits = bits,
                Name = property
            };

            component.PropertyChanged += (a, b) => {
                // only call OnPropertyChanged when the requested property changes
                if(b.PropertyName != property) {
                    return;
                }

                var val = component.GetType().GetProperty(property).GetValue(component, null);

                // ensure that it is ushort
                if(value.GetType().Name != nameof(UInt16)) {
                    Log.Warn($"Input property, {property} is not a valid type (ushort)");
                    return;
                }
                inp.Value = (ushort)val;
                OnPropertyChanged("Inputs");
            };

            Inputs.Add(inp);
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
                            if(changed == depended) {
                                graph.AddEdge(command, comm);
                            }
                        }
                    }
                }
            }
            ActiveGraph = graph;
        }
    }

    // observable class to store 3 values ...
    // why does this have to be so long?
    public class MicrocodeInput : ObservableObject {

        // value to wrap in this class
        private ushort _value;
        public ushort Value {
            get { return _value; }
            set {
                _value = value;
                OnPropertyChanged();
            }
        }


        // take the lower {bits} bits of the value
        private int _bits;
        public int Bits {
            get { return _bits; }
            set {
                _bits = value;
                OnPropertyChanged();
            }
        }

        // name of the input
        private string _name;
        public string Name {
            get { return _name; }
            set {
                _name = value;
                OnPropertyChanged();
            }
        }
    }
}
