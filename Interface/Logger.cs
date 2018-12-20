using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Controls;
using VMLib;

namespace Interface {
    class Logger : ILogger {
        public ObservableCollection<string> Log;

        public Logger(ref ObservableCollection<string> log) {
            Log = log;
        }

        public void Info(string message) {
            Log.Add($"[{DateTime.Now.ToString()}] INFO: {message}");
        }

        public void Warn(string message) {
            Log.Add($"[{DateTime.Now.ToString()}] WARN: {message}");
        }

        public void Error(string message) {
            Log.Add($"[{DateTime.Now.ToString()}] ERROR: {message}");
        }
    }
}
