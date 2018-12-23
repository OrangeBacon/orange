using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Interface.Framework;
using Interface;
using System.Windows;
using System.Windows.Input;
using VMLib;

namespace Interface.ViewModels {
    class MicrocodeProcess : ObservableObject {

        public VMCore VM { get; } = new StarfishVM(Globals.Log).VM;

        public event EventHandler ShowLogEvent;

        private ICommand _showLog;
        public ICommand ShowLog {
            get {
                if (_showLog == null) {
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
            foreach (var command in ActiveCommands) {
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
            if (ActiveCommands.Contains(command)) {
                ActiveCommands.Remove(command);
            } else {
                ActiveCommands.Add(command);
            }
        }
    }
}
