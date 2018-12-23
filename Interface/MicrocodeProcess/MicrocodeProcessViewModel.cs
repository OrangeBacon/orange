using Interface.Framework;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Collections.Specialized;
using System.Linq;
using System.Reflection;
using System.Windows.Input;
using VMLib;

namespace Interface.ViewModels {
    internal class MicrocodeProcess : ObservableObject {
        public MicrocodeProcess() {
            ((INotifyCollectionChanged)VM.Components).CollectionChanged += UpdateComponents;
            UpdateComponents(null, new NotifyCollectionChangedEventArgs(NotifyCollectionChangedAction.Add, VM.Components));
        }

        public VMCore VM { get; } = new StarfishVM(Globals.Log).VM;

        public event EventHandler ShowLogEvent;

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

        private ICommand _singleStep;
        public ICommand SingleStep {
            get {
                if(_singleStep == null) {
                    _singleStep = new RelayCommand(SingleStepExecute);
                }
                return _singleStep;
            }
        }
        private void SingleStepExecute() {
            Globals.Log.Info("STEP");
            foreach(var command in ActiveCommands) {
                Globals.Log.Info(command.Name);
            }
        }

        private ICommand _checkBox;
        public ICommand CheckBox {
            get {
                if(_checkBox == null) {
                    _checkBox = new RelayCommand(CheckBoxExecute);
                }
                return _checkBox;
            }
        }
        private readonly List<MicrocodeCommand> ActiveCommands = new List<MicrocodeCommand>();
        private void CheckBoxExecute(object e) {
            Globals.Log.Info("Click");
            var command = e as MicrocodeCommand;
            if(ActiveCommands.Contains(command)) {
                ActiveCommands.Remove(command);
            } else {
                ActiveCommands.Add(command);
            }
        }

        public ObservableCollection<ComponentView> Components { get; } = new ObservableCollection<ComponentView>();
        private void UpdateComponents(object sender, NotifyCollectionChangedEventArgs e) {
            foreach(var item in e.NewItems) {
                var component = item as Component;

                var properties = new List<Int32Prop>();
                foreach(var prop in component.GetType().GetProperties()) {
                    if(!typeof(Component).GetProperties().Contains(prop) && prop.GetValue(component).GetType() == typeof(short)) {
                        properties.Add(new Int32Prop() {
                            Value = (short)prop.GetValue(component),
                            Name = prop.Name,
                        });
                    }
                }

                var view = new ComponentView() {
                    Name = component.Name,
                    ID = component.ComponentID,
                    TypeName = component.TypeName,
                    Properties = new ObservableCollection<Int32Prop>(properties)
                };

                component.PropertyChanged += (propSender, propE) => {
                    foreach(var prop in properties) {
                        if(prop.Name == propE.PropertyName) {
                            foreach(var p in propSender.GetType().GetProperties()) {
                                if(p.Name == prop.Name) {
                                    prop.Value = (short)p.GetValue(propSender);
                                }
                            }
                        }
                    }
                };


                Components.Add(view);
            }
        }
    }

    internal class Int32Prop : ObservableObject {
        private short _value;
        public short Value { get { return _value; } set { _value = value; OnPropertyChanged(); } }
        public string Name { get; set; }
    }

    internal class ComponentView {
        public string Name { get; set; }
        public int ID { get; set; }
        public string TypeName { get; set; }
        public ObservableCollection<Int32Prop> Properties { get; set; }
    }
}
