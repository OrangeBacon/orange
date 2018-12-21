using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Interface.Framework;
using Interface;
using System.Windows;
using System.Windows.Input;

namespace Interface.ViewModels {
    class MicrocodeProcess : ObservableObject {
        public event EventHandler ShowLogEvent;

        private ICommand _showLog;
        public ICommand ShowLog {
            get {
                if (_showLog == null) {
                    _showLog = new RelayCommand(() => {
                        ShowLogEvent?.Invoke(this, new EventArgs());
                    });
                }
                return _showLog;
            }
        }
    }
}
