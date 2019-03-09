using System;
using System.Windows.Input;

namespace Interface.Framework {

    // wrapper around the windows command interface to make it 
    // slightly less painful to use.
    internal class RelayCommand : ICommand {
        private readonly Action<object> _action;
        private readonly Predicate<object> _canExecute;

        // overloads of the constructor
        public RelayCommand(Action action) : this(_ => action(), null) { }

        public RelayCommand(Action<object> action) : this(action, null) { }

        public RelayCommand(Action action, Predicate<object> canExecute) : this(_ => action(), canExecute) { }

        public RelayCommand(Action<object> action, Predicate<object> canExecute) {
            // actions are required, canExecute is optional
            _action = action ?? throw new ArgumentNullException("action");
            _canExecute = canExecute;
        }

        // run by the interface to check if the command is able to be run
        public bool CanExecute(object parameter) {
            return _canExecute == null ? true : _canExecute(parameter);
        }

        // run the command
        public void Execute(object parameter) {
            _action(parameter);
        }

        public event EventHandler CanExecuteChanged {
            add { CommandManager.RequerySuggested += value; }
            remove { CommandManager.RequerySuggested -= value; }
        }
    }
}
