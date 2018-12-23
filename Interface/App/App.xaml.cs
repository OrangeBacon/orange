using System;
using System.Collections.Generic;
using System.Configuration;
using System.Data;
using System.Linq;
using System.Threading.Tasks;
using System.Windows;
using Squirrel;

namespace Interface {
    /// <summary>
    /// Interaction logic for App.xaml
    /// </summary>
    public partial class App : Application {
        private void Application_Startup(object sender, StartupEventArgs e) {
            Task.Run(Update);
            var win = new Views.MicrocodeProcess {
                Title = "Starfish Virtual Machine"
            };
            win.Show();
        }

        private async Task Update() {
            using (var mgr = UpdateManager.GitHubUpdateManager(@"https://github.com/ScratchOs/starfish")) {
                Framework.Globals.Log.Info("Starting Update Check");
                await mgr.Result.UpdateApp();
                Framework.Globals.Log.Info("Finished Update Check");
            }
        }
    }
}
