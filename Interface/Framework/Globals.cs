using System;
using System.Collections.ObjectModel;

namespace Interface.Framework {
    // global variables!  (currently only the log)
    static class Globals {
        public static Logger Log = new Logger();

        // implement the logger
        public class Logger : VMLib.ILogger {
            static public ObservableCollection<string> LogText { get; } = new ObservableCollection<string>();

            public void Info(string message) {
                App.Current.Dispatcher.Invoke(() => {
                    LogText.Add($"[{DateTime.Now.ToString()}] INFO: {message}");
                });
            }

            public void Warn(string message) {
                App.Current.Dispatcher.Invoke(() => {
                    LogText.Add($"[{DateTime.Now.ToString()}] WARN: {message}");
                });
            }

            public void Error(string message) {
                App.Current.Dispatcher.Invoke(() => {
                    LogText.Add($"[{DateTime.Now.ToString()}] ERROR: {message}");
                });
            }
        }
    }
}
