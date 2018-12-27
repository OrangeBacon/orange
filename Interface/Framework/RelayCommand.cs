using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Input;

namespace Interface.Framework {
    internal class RelayCommand : ICommand {
        private readonly Action<object> _action;
        private readonly Predicate<object> _canExecute;

        public RelayCommand(Action action) : this(_ => action(), null) { }

        public RelayCommand(Action<object> action) : this(action, null) { }

        public RelayCommand(Action action, Predicate<object> canExecute) : this(_ => action(), canExecute) { }

        public RelayCommand(Action<object> action, Predicate<object> canExecute) {
            _action = action ?? throw new ArgumentNullException("action");
            _canExecute = canExecute;
        }

        public bool CanExecute(object parameter) {
            return _canExecute == null ? true : _canExecute(parameter);
        }

        public void Execute(object parameter) {
            _action(parameter);
        }

        public event EventHandler CanExecuteChanged {
            add { CommandManager.RequerySuggested += value; }
            remove { CommandManager.RequerySuggested -= value; }
        }
    }
}
