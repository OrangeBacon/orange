using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Input;

namespace Interface.Framework {
    class DelagateCommand : ICommand {
        private readonly Action _action;
        private readonly Func<bool> _canExecute = () => true;

        public DelagateCommand(Action action) {
            _action = action;
        }

        public DelagateCommand(Action action, Func<bool> canExecute) {
            _action = action;
            _canExecute = canExecute;
        }

        public bool CanExecute(object parameter) {
            return _canExecute();
        }

        public void Execute(object parameter) {
            _action();
        }

        public event EventHandler CanExecuteChanged;

        public void RaiseCanExecuteChanged() {
            CanExecuteChanged?.Invoke(this, new EventArgs());
        }
    }
}
