using Interface.Framework;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Collections.Specialized;
using System.Linq;
using System.Windows.Input;
using VMLib;
using static VMLib.StarfishVM;

namespace Interface.ViewModels {
    internal class MicrocodeProcess : ObservableObject {
        public MicrocodeProcess() {
            // when a new component is added to the emulator run UpdateComponents
            ((INotifyCollectionChanged)VM.Components).CollectionChanged += UpdateComponents;

            // force initial update with current components
            UpdateComponents(null, new NotifyCollectionChangedEventArgs(NotifyCollectionChangedAction.Add, VM.Components));
        }

        // Main VM used in all execution
        public VMCore VM { get; } = CreateStarfishVM(Globals.Log);

        // fired when the user wants to view the log screen
        public event EventHandler ShowLogEvent;

        // show log command
        private ICommand _showLog;
        public ICommand ShowLog {
            get {
                if(_showLog == null) {
                    _showLog = new RelayCommand(ShowLogExecute);
                }
                return _showLog;
            }
        }
        private void ShowLogExecute() {
            ShowLogEvent?.Invoke(this, new EventArgs());
        }

        // run one step on the vm command
        private ICommand _singleStep;
        public ICommand SingleStep {
            get {
                if(_singleStep == null) {
                    _singleStep = new RelayCommand(SingleStepExecute, CanSingleStepExecute);
                }
                return _singleStep;
            }
        }
        private bool CanSingleStepExecute(object obj) {
            // only allowed to execute when the graph works or it is a no op
            return VM.Controller.ActiveGraph.TopologicalSort().Count != 0 || VM.Controller.ActiveCommands.Count == 0;
        }
        private void SingleStepExecute() {
            Globals.Log.Info("STEP");
            VM.Clock.RunCycle();
        }

        // command to enable / disable a specific microcode command
        private ICommand _checkBox;
        public ICommand CheckBox {
            get {
                if(_checkBox == null) {
                    _checkBox = new RelayCommand(CheckBoxExecute);
                }
                return _checkBox;
            }
        }
        private void CheckBoxExecute(object e) {
            var command = e as MicrocodeCommand;
            if(VM.Controller.ActiveCommands.Contains(command)) {
                VM.Controller.ActiveCommands.Remove(command);
            } else {
                VM.Controller.ActiveCommands.Add(command);
            }
        }

        // list of all the data required to display each component
        // read by the user interface
        public ObservableCollection<ComponentModel> Components { get; } = new ObservableCollection<ComponentModel>();

        // updates the user interface with all of the new components
        // TODO: deal with removing components from the emulator,
        // however until devices are implemented, it will not be useful.
        private void UpdateComponents(object sender, NotifyCollectionChangedEventArgs e) {

            // loop through new components
            foreach(var item in e.NewItems) {
                var component = item as Component;
                
                // extract all displayable properties of the component - ie all ushort values
                var properties = new List<UInt16Prop>();
                foreach(var prop in component.GetType().GetProperties()) {
                    if(!typeof(Component).GetProperties().Contains(prop) && prop.GetValue(component).GetType() == typeof(ushort)) {
                        properties.Add(new UInt16Prop() {
                            Value = (ushort)prop.GetValue(component),
                            Name = prop.Name,
                        });
                    }
                }

                // create the model for this component
                var view = new ComponentModel() {
                    Name = component.Name,
                    ID = component.ComponentID,
                    TypeName = component.TypeName,
                    Properties = new ObservableCollection<UInt16Prop>(properties)
                };

                // allow the user interface to update when each property changes
                component.PropertyChanged += (propSender, propE) => {

                    // loop through the properties on the component
                    foreach(var prop in properties) {

                        // i've forgotten how this bit works?
                        if(prop.Name == propE.PropertyName) {
                            foreach(var p in propSender.GetType().GetProperties()) {
                                if(p.Name == prop.Name) {
                                    prop.Value = (ushort)p.GetValue(propSender);
                                }
                            }
                        }
                    }
                };

                // add the component to the user interface
                Components.Add(view);
            }
        }
    }

    // wrapper around a single property on a component
    internal class UInt16Prop : ObservableObject {
        private ushort _value;
        public ushort Value { get { return _value; } set { _value = value; OnPropertyChanged(); } }
        public string Name { get; set; }
    }

    // rearranged layout of a component to make it easier to display
    internal class ComponentModel {
        public string Name { get; set; }
        public int ID { get; set; }
        public string TypeName { get; set; }
        public ObservableCollection<UInt16Prop> Properties { get; set; }
    }
}
