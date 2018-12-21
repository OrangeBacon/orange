using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Collections.ObjectModel;

namespace Interface.ViewModels {
    public class App : Framework.ObservableObject, VMLib.ILogger {
        public static string TypeName => nameof(App);

        private readonly Models.App appModel = new Models.App();

        public void Info(string message) {
            appModel.Log.Add($"[{DateTime.Now.ToString()}] INFO: {message}");
        }

        public void Warn(string message) {
            appModel.Log.Add($"[{DateTime.Now.ToString()}] WARN: {message}");
        }

        public void Error(string message) {
            appModel.Log.Add($"[{DateTime.Now.ToString()}] ERROR: {message}");
        }

        public ObservableCollection<string> Log { get => appModel.Log; }
    }
}
