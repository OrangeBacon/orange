using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Controls;
using VMLib;

namespace Interface {
    class Logger : ILogger {
        readonly ItemsControl list;
        readonly List<string> logItems = new List<string>();

        public Logger(ItemsControl list) {
            this.list = list;
            list.ItemsSource = logItems;
        }

        public void Info(string message) {
            logItems.Add($"[{DateTime.Now.ToString()}] INFO: {message}");
        }

        public void Warn(string message) {
            logItems.Add($"[{DateTime.Now.ToString()}] WARN: {message}");
        }

        public void Error(string message) {
            logItems.Add($"[{DateTime.Now.ToString()}] ERROR: {message}");
        }
    }
}
