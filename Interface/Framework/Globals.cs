using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Interface.Framework {
    static class Globals {
        public static Logger Log = new Logger();

        public class Logger : VMLib.ILogger {
            static public ObservableCollection<string> LogText { get; } = new ObservableCollection<string>();

            public void Info(string message) {
                LogText.Add($"[{DateTime.Now.ToString()}] INFO: {message}");
            }

            public void Warn(string message) {
                LogText.Add($"[{DateTime.Now.ToString()}] WARN: {message}");
            }

            public void Error(string message) {
                LogText.Add($"[{DateTime.Now.ToString()}] ERROR: {message}");
            }
        }
    }
}
