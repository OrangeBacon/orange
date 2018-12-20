using System;
using System.Collections.Generic;
using System.Configuration;
using System.Data;
using System.Linq;
using System.Threading.Tasks;
using System.Windows;

namespace Interface {
    /// <summary>
    /// Interaction logic for App.xaml
    /// </summary>
    public partial class App : Application {
        App() {
            Current.Properties.Add(Models.App.Name, new Models.App());
        }

        private void Application_Startup(object sender, StartupEventArgs e) {
            var win = new Views.MicrocodeProcess();
            win.Title = "Starfish Virtual Machine";
            win.Show();
        }
    }
}
